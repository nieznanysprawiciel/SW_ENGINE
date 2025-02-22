#include "EngineEditor/LightmapsTool/stdafx.h"
#include "LightmapWorkerCPU.h"
#include "EngineCore/ModelsManager/DefaultAssets.h"

#include <cstring>
#include <limits>
// Trzeba oddefiniowa�, je�eli chce si� korzysta� z numeric_limits
#undef max

#include "Common/MemoryLeaks.h"

using namespace DirectX;

LightmapWorkerCPU::LightmapWorkerCPU( SceneData* sceneData )
	: LightmapWorker( sceneData )
{
	m_threshold = 0.04f;
}

// ============================================================================= //
// Helpers
// ============================================================================= //

/**@brief Szybsze (miejmy nadziej�) mno�enie przez 3.*/
inline unsigned int mul3( unsigned int number )
{	return ( number << 1 ) + number;	}

/**@brief Dodaje pod podanymi indeksami w chunku element addValue.*/
inline XMFLOAT3& LoadAddStore( std::vector<MemoryChunk>& chunk, unsigned int i, unsigned int j, XMVECTOR addValue )
{
	auto& lightValueRef = chunk[ i ].Get<XMFLOAT3>( j );
	XMVECTOR lightValue = XMLoadFloat3( &lightValueRef );
	lightValue = XMVectorAdd( addValue, lightValue );
	XMStoreFloat3( &lightValueRef, lightValue );
	
	return lightValueRef;
}


inline void CompareEmission( std::tuple<unsigned int, unsigned int, float>& prevEmission, unsigned int i, unsigned int j, const XMFLOAT3& newEmission )
{
	if( newEmission.x > std::get<2>( prevEmission ) )
		prevEmission = std::make_tuple( i, j, newEmission.x );
	if( newEmission.y > std::get<2>( prevEmission ) )
		prevEmission = std::make_tuple( i, j, newEmission.y );
	if( newEmission.z > std::get<2>( prevEmission ) )
		prevEmission = std::make_tuple( i, j, newEmission.z );
}

std::tuple<unsigned int, unsigned int, float> FindMaxEmission( std::vector<MemoryChunk>& emissionLight )
{
	std::tuple<unsigned int, unsigned int, float> emissionMax = std::make_tuple( 0, 0, -1.0f );
	for( unsigned int i = 0 ; i < emissionLight.size(); ++i )
		for( unsigned int j = 0; j < emissionLight[ i ].Count<XMFLOAT3>(); ++j )
		{
			auto& emissionVector = emissionLight[ i ].Get<XMFLOAT3>( j );
			CompareEmission( emissionMax, i, j, emissionVector );
		}
	return emissionMax;
}

/**@brief Czy�ci bufory g��boko�ci i indeks�w.*/
void ClearDepthIndex( float* depth, BufferIndexing* index, unsigned int elements )
{
	for( unsigned int i = 0; i < elements; ++i )
		depth[ i ] = std::numeric_limits<float>::max();
	for( unsigned int i = 0; i < elements; ++i )
	{
		index[ i ].first = INVALID_INDEX;
		index[ i ].second = INVALID_INDEX;
	}
}

/**@brief Maksymalna warto�� trzech liczb.*/
int max3( float x1, float x2, float x3 )
{
	if( x1 > x2 )
		return x1 > x3 ? (int)x1 : (int)x3;
	else
		return x2 > x3 ? (int)x2 : (int)x3;
}

/**@brief Minimalna warto�� trzech liczb.*/
int min3( float x1, float x2, float x3 )
{
	if( x1 < x2 )
		return x1 < x3 ? (int)x1 : (int)x3;
	else
		return x2 < x3 ? (int)x2 : (int)x3;
}


// ============================================================================= //
// Generation
// ============================================================================= //

/**@brief G��wna funkcja generuj�ca lightmapy. Zaimplementuj w klasie pochodnej.*/
void LightmapWorkerCPU::Generate		()
{
	if( m_data->objectParts.empty() )
		return;

	// Chunki dla ca�ych obiek�w (nie dla podcz�ci)
	std::vector<MemoryChunk> emissionLight;
	std::vector<MemoryChunk> reachedLight;
	std::vector<MemoryChunk> verticies;			// Przekszta�cone wierzcho�ki i normalne.

	Prepare( emissionLight, reachedLight, verticies );
	Radiosity( emissionLight, reachedLight, verticies );
	BuildResult( reachedLight );

	m_lightmapState = LightmapState::GenerationEnded;		// Ustaw na ko�cu funkcji, �eby w�tek g��wny m�g� zebra� wyniki.
}

/**@brief Przygotowuje odpowiednie struktury do generacji.*/
void LightmapWorkerCPU::Prepare( std::vector<MemoryChunk>& emissionLight, std::vector<MemoryChunk>& reachedLight, std::vector<MemoryChunk>& verticies )
{
	auto& parts = m_data->objectParts;
	for( auto& part : parts )
	{
		unsigned int chunkSize = part.verticesCount * sizeof( XMFLOAT3 ) / 3;

		MemoryChunk emissionChunk( chunkSize );
		MemoryChunk reachedChunk( chunkSize );
		MemoryChunk verticiesChunk( part.verticesCount * sizeof( VertexFormat ) );

		memset( reachedChunk.GetMemory<void>(), 0, chunkSize );
		memset( emissionChunk.GetMemory<void>(), 0, chunkSize );		// Potem wype�nimy warto�ciami emisji w p�tli.

		XMMATRIX transformMatrix = XMLoadFloat4x4( &part.transform );

		// Transform positions
		XMVector3TransformCoordStream(	&verticiesChunk.Get<VertexFormat>( 0 ).position,
										sizeof( VertexFormat ),
										&m_data->verticies[ part.chunkIdx ].Get<VertexNormalTexCoord>( part.bufferOffset ).Position,
										sizeof( VertexNormalTexCoord ),
										part.verticesCount,
										transformMatrix );

		// Transform normals
		XMVector3TransformNormalStream( &verticiesChunk.Get<VertexFormat>( 0 ).normal,
										sizeof( VertexFormat ),
										&m_data->verticies[ part.chunkIdx ].Get<VertexNormalTexCoord>( part.bufferOffset ).Normal,
										sizeof( VertexNormalTexCoord ),
										part.verticesCount,
										transformMatrix );

		if( part.emissive.x != 0.0f || part.emissive.y != 0.0f || part.emissive.z != 0.0f )
		{
			XMFLOAT3 materialEmissive( part.emissive.x, part.emissive.y, part.emissive.z );

			unsigned int trianglesCount = part.verticesCount / 3;

			// To takie skalowanie na oko troszk�. W ka�dym razie zak�adamy, �e �wiat�o o mocy (1,1,1) powinno wypromieniowa�
			// tyle samo energii niezale�nie od ilo�ci wierzcho�k�w. W przysz�o�ci trzeba b�dzie jako� pewnie uwzgl�dnia� powierzchni�
			// tr�jk�t�w zar�wno emituj�cych jak i odbieracj�cych. Tych oblicze� widz� robi si� coraz wi�cej.
			//float scaleFactor = 1.0f/*(float)m_depthResolution *// (float)trianglesCount;
			//materialEmissive.x = materialEmissive.x * scaleFactor;
			//materialEmissive.y = materialEmissive.y * scaleFactor;
			//materialEmissive.z = materialEmissive.z * scaleFactor;
			
			for( unsigned int k = 0; k < trianglesCount; ++k )
				emissionChunk.Get<XMFLOAT3>( k ) = materialEmissive;
		}

		emissionLight.push_back( std::move( emissionChunk ) );
		reachedLight.push_back( std::move( reachedChunk ) );
		verticies.push_back( std::move( verticiesChunk ) );
	}
}


/**@brief Wykonuje algorytm radiosity.*/
void LightmapWorkerCPU::Radiosity( std::vector<MemoryChunk>& emissionLight,
								   std::vector<MemoryChunk>& reachedLight,
								   std::vector<MemoryChunk>& verticies )
{
	// Identyfikuje wielok�t z najwi�ksz� energi� do wyemitowania.
	std::tuple<unsigned int, unsigned int, float> emissionMax = FindMaxEmission( emissionLight );

	//std::get<1>( emissionMax ) = 13;		// Testttttt

	// Bufor g��boko�ci i bufor s�u��cy do indeksowania tr�jk�t�w, kt�re znajduj� si� w buforze.
	unsigned int		depthSize	= m_depthResolution * m_depthResolution;
	MemoryChunk			depthBuffer( depthSize * sizeof( float) );
	MemoryChunk			indexBuffer( depthSize * sizeof( BufferIndexing ) );

	// Ko�czymy generowanie, gdy najwi�ksza zgromadzona w wielok�cie energia spadnie poni�ej pewnego poziomu.
	int iterations = 0;	
	/*while( std::get<2>( emissionMax ) > m_threshold )*/
	/*while( std::get<2>( emissionMax ) < 1 )*/
	while( iterations < 30 )
	{
		iterations++;

		// Czy�cimy bufory
		ClearDepthIndex( depthBuffer.GetMemory<float>(), indexBuffer.GetMemory<BufferIndexing>(), depthSize );

		DepthPass( emissionMax, verticies, depthBuffer, indexBuffer );
		TransferPass( emissionMax, verticies, emissionLight, reachedLight, indexBuffer );

		emissionMax = FindMaxEmission( emissionLight );
	}
}

/**@brief Tworzy tablic� color - uv, kt�ra zostanie potem u�yta do wyrenderowania lightmapy.

Wszystkie chunki podzielone wcze�niej na podcz�ci s� teraz ��czone. Ka�dy obiekt b�dzie mia� sw�j bufor.*/
void LightmapWorkerCPU::BuildResult	( std::vector<MemoryChunk>& reachedLight )
{
	auto& parts = m_data->objectParts;
	auto object = parts[0].object;
	unsigned int verticiesCount = 0;
	unsigned int firstPartIndex = 0;

	for( unsigned int i = 0; i < parts.size(); ++i )
	{
		verticiesCount += parts[i].verticesCount;

		// Sumujemy liczb� wierzcho�k�w w obiekcie. Dopiero jak zacznie si� nowy obiekt to tworzymy bufor.
		unsigned int nextIdx = i+1;
		if( nextIdx == parts.size() || parts[nextIdx].object != object )	// Pierwszy warunek wymusza, �e we�miemy tak�e ostatni obiekt
		{
			if( verticiesCount != 0 )
			{
				MemoryChunk colorMap( verticiesCount * sizeof( CoordColor ) );

				unsigned int verticiesOffset = 0;
				for( unsigned int j = firstPartIndex; j < nextIdx; ++j )
				{
					for( unsigned int k = 0; k < reachedLight[ j ].Count<XMFLOAT3>(); ++k )
					{
						CoordColor& colorVertex1 = colorMap.Get<CoordColor>( verticiesOffset + mul3( k ) );		// Jest 3 razy wi�cej wierzcho�k�w ni� kolor�w.
						XMFLOAT3& lightColor = reachedLight[ j ].Get<XMFLOAT3>( k ); /*XMFLOAT3( 0.5, 0.3, 0.7);*/
						colorVertex1.Color = lightColor;
						colorVertex1.TexCoord = m_data->verticies[ parts[j].chunkIdx ].Get<VertexNormalTexCoord>( parts[j].bufferOffset + mul3( k ) ).TexCoord;

						CoordColor& colorVertex2 = colorMap.Get<CoordColor>( verticiesOffset + mul3( k ) + 1 );
						colorVertex2.Color = lightColor;
						colorVertex2.TexCoord = m_data->verticies[ parts[j].chunkIdx ].Get<VertexNormalTexCoord>( parts[j].bufferOffset + mul3( k ) + 1 ).TexCoord;

						CoordColor& colorVertex3 = colorMap.Get<CoordColor>( verticiesOffset + mul3( k ) + 2 );
						colorVertex3.Color = lightColor;
						colorVertex3.TexCoord = m_data->verticies[ parts[j].chunkIdx ].Get<VertexNormalTexCoord>( parts[j].bufferOffset + mul3( k ) + 2 ).TexCoord;
					}

					verticiesOffset += parts[j].verticesCount;
				}

				m_resultData.push_back( std::move( colorMap ) );
			}

			if( nextIdx == parts.size() )		break;		// Nie ma wi�cej obiekt�w.
			// Zako�czyli�my przetwarzanie obiektu przechodzimy do nast�pnego.
			object = parts[nextIdx].object;
			firstPartIndex = nextIdx;
			verticiesCount = 0;
		}
			
	}
}

/**@brief Wykonuje przebieg z z-buforem.*/
void LightmapWorkerCPU::DepthPass( std::tuple<unsigned int, unsigned int, float>& emissionMax,
								   std::vector<MemoryChunk>& verticies,
								   MemoryChunk& depthBuffer,
								   MemoryChunk& indexBuffer )
{
	// Pobieramy indeksy emitera.
	unsigned int idx1 = std::get<0>( emissionMax );
	unsigned int idx2 = std::get<1>( emissionMax );

	uint32 depthFailCounter = 0;
	uint32 triangleCounter = 0;

	// Liczmymy pozycj� emitera, u�rednion� normaln� i uk�ad wsp�rz�dnych zwi�zany z emiterem dla z-bufora.
	Triangle4 emiterPosition( &verticies[ idx1 ].Get<VertexFormat>( mul3( idx2 ) ) );
	//Triangle4 emiterCoordSystem = EmiterCoordinatesSystem( emiterPosition );
	//XMVECTOR emiterNormal = AverageNormal( &verticies[ idx1 ].Get<VertexFormat>( mul3( idx2 ) ) );	// Mo�liwe, �e niepotrzebne. emiterCoordSystem ma normaln�, ale wyliczon� inaczej.
	XMMATRIX emiterViewMatrix = EmiterViewMatrix( emiterPosition );

	for( unsigned int i = 0 ; i < verticies.size(); ++i )
	{
		for( unsigned int j = 0; j < verticies[ i ].Count<VertexFormat>() / 3; ++j )
		{
			if( idx2 == j && idx1 == i )
				continue;	// Nie pr�bujmy rzutowa� emitera na samego siebie.

			triangleCounter++;

			// Wyliczamy pozycj� odbiorcy w uk�adzie wsp�rz�dnych bufora g��boko�ci.
			Triangle4 receiverPosition( &verticies[ i ].Get<VertexFormat>( mul3( j ) ) );
			XMFLOAT3 receiverDepths = HemisphereCast( emiterPosition, receiverPosition, emiterViewMatrix );
			if( receiverDepths.x < 0.0f || receiverDepths.y < 0.0f || receiverDepths.z < 0.0f )
			{
				depthFailCounter++;
				continue;		// Tr�jk�t jest cz�ciowo za p�aszczyzn�. Musimy go odrzuci�.
			}

			HemisphereViewport( receiverPosition );
			RasterizeTriangle( receiverPosition, &receiverDepths, i, j, depthBuffer, indexBuffer );
		}

	}
}

void LightmapWorkerCPU::TransferPass( std::tuple<unsigned int, unsigned int, float>& emissionMax,
									  std::vector<MemoryChunk>& verticies,
									  std::vector<MemoryChunk>& emissionLight,
									  std::vector<MemoryChunk>& reachedLight,
									  MemoryChunk& indexBuffer )
{
	// Pobieramy indeks emitera i zerujemy maksymaln� emisj�. B�dziemy szuka� nowej
	// maksymalnej warto�ci w tym przebiegu.
	std::get<2>( emissionMax ) = 0.0f;
	unsigned int idx1 = std::get<0>( emissionMax );
	unsigned int idx2 = std::get<1>( emissionMax );

	unsigned int depthSize = m_depthResolution * m_depthResolution;

	// Wyliczamy ile energii powinien dosta� ka�dy element, kt�ry znajdziemy w buforze indeks�w.
	XMVECTOR depthElementWeight = XMVectorReplicate( static_cast<float>( 50.0 * 4.0 * XM_1DIV2PI / depthSize ) );
	XMVECTOR emitedLight = XMLoadFloat3( &emissionLight[ idx1 ].Get<XMFLOAT3>( idx2 ) );
	emitedLight = XMVectorMultiply( emitedLight, depthElementWeight );

	// Zerujemy �wiat�o, kt�re emiter b�dzie m�g� wyemitowa� w kolejnych iteracjach.
	XMStoreFloat3( &emissionLight[ idx1 ].Get<XMFLOAT3>( idx2 ), XMVectorZero() );

	for( unsigned int i = 0; i < depthSize; ++i )
	{
		BufferIndexing indicies = indexBuffer.Get<BufferIndexing>( i );
		// Tylko piksele na kole powinny dosta� dawk� energii, a bufor jest kwadratem.
		// Dlatego elementy, na kt�re nie zosta� zrzutowany �aden tr�jk�t zosta�y zainicjowane warto�ci� INVALID_INDEX.
		if( indicies.first != INVALID_INDEX )
		{
			XMVECTOR materialDiffuse = XMLoadFloat4( &m_data->objectParts[ indicies.first ].diffuse );
			XMVECTOR receivedLight = XMVectorMultiply( materialDiffuse, emitedLight );
			
			//receivedLight = XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f );		// shitty test

			const XMFLOAT3& emissionPower = LoadAddStore( emissionLight, indicies.first, indicies.second, receivedLight );
			LoadAddStore( reachedLight, indicies.first, indicies.second, receivedLight );
			//CompareEmission( emissionMax, indicies.first, indicies.second, emissionPower );
		}
	}
	//// another shitty test
	//XMVECTOR receivedLight = XMVectorSet( 1.0f, 1.0f, 1.0f, 1.0f );
	//LoadAddStore( reachedLight, idx1, idx2, receivedLight );
}


/**@brief Wylicza wsp�rz�dne tr�jk�ta zrzutowanego na p�aszczyzn� ko�a.
Wsp�rz�dne s� dwuwymiarowe, co pozwala �atwo przej�� z nich na indeksy do bufora
g��boko�ci.

@param[inout] receiver Tr�jk�t do zrzutowania. W tej zmiennej zwracany jest tr�jk�t wynikowy.
@return Zwraca wektor z g��boko�ciami dla ka�dego tr�jk�ta. Je�eli jaki� wierzcho�ek
znajduje si� za p�aszczyzn� rzutowania, funkcja zwraca (-1.0, -1.0, -1.0). W normalnej sytuacji
wszystkie wsp�rz�dne s� dodatnie.*/
DirectX::XMFLOAT3 LightmapWorkerCPU::HemisphereCast( Triangle4& emiter, Triangle4& receiver, XMMATRIX& emiterViewMatrix )
{
	// Poniewa� to s� wierzcho�ki, to ustawiamy komponent w na 1.
	receiver.vertex1 = XMVectorSetW( receiver.vertex1, 1.0f );
	receiver.vertex2 = XMVectorSetW( receiver.vertex2, 1.0f );
	receiver.vertex3 = XMVectorSetW( receiver.vertex3, 1.0f );

	// Transformujemy wierzcho�ki do uk�adu emitera. Dzi�ki temu xy jest pozycj� na p�aszczy�nie (na razie jeszcze nie jednostkowej)
	// a z jest odleg�o�ci� od emitera.
	XMVECTOR centerToVertex1 = XMVector4Transform( receiver.vertex1, emiterViewMatrix );
	XMVECTOR centerToVertex2 = XMVector4Transform( receiver.vertex2, emiterViewMatrix );
	XMVECTOR centerToVertex3 = XMVector4Transform( receiver.vertex3, emiterViewMatrix );

	// Wyliczenie d�ugo�ci od �rodka do wierzcho�k�w.
	XMVECTOR depth1 = XMVector3Length( centerToVertex1 );
	XMVECTOR depth2 = XMVector3Length( centerToVertex2 );
	XMVECTOR depth3 = XMVector3Length( centerToVertex3 );

	// Normalizacja wektor�w
	receiver.vertex1 = XMVectorDivide( centerToVertex1, depth1 );
	receiver.vertex2 = XMVectorDivide( centerToVertex2, depth2 );
	receiver.vertex3 = XMVectorDivide( centerToVertex3, depth3 );

	XMFLOAT3 depthVector;
	
	depthVector.x = -XMVectorGetZ( centerToVertex1 );
	depthVector.y = -XMVectorGetZ( centerToVertex2 );
	depthVector.z = -XMVectorGetZ( centerToVertex3 );

	//if( XMVectorGetZ( centerToVertex1 ) > 0 )
	//	depthVector.x = -XMVectorGetX( depth1 );
	//else
	//	depthVector.x = XMVectorGetX( depth1 );

	//if( XMVectorGetZ( centerToVertex2 ) > 0 )
	//	depthVector.y = -XMVectorGetX( depth2 );
	//else
	//	depthVector.y = XMVectorGetX( depth2 );
	//
	//if( XMVectorGetZ( centerToVertex3 ) > 0 )
	//	depthVector.z = -XMVectorGetX( depth3 );
	//else
	//	depthVector.z = XMVectorGetX( depth3 );

	return depthVector;
}

/**@brief Wyliczamy wektory r�wnoleg�e do p�aszczyzny i wzajemnie ortogonalne.

Pos�u�� one za uk�ad wsp�rz�dnych, w kt�rym b�dzie wyra�ony rzut tr�jk�ta.
W zasadzie ich orientacja jest dowolna, wa�ne �eby by�y stosowane te same
wektory do wszystkich rzut�w.*/
Triangle4 LightmapWorkerCPU::EmiterCoordinatesSystem( Triangle4& emiter )
{
	XMVECTOR edge12 = XMVectorSubtract( emiter.vertex2, emiter.vertex1 );
	XMVECTOR edge13 = XMVectorSubtract( emiter.vertex3, emiter.vertex1 );

	// Wyliczamy najpierw wektor normalny do p�aszczyzny, z potem tworzymy 2 kolejne wektory ortogonalne.
	Triangle4 CoordSystem;
	CoordSystem.vertex1 = XMVector3Normalize( XMVector3Cross( edge13, edge12 ) );
	CoordSystem.vertex2 = XMVector3Normalize( edge12 );															// Jedn� z osi wybieramy totalnie dowolnie, wa�ne by by�a na p�aszczy�nie.
	CoordSystem.vertex3 = XMVector3Normalize( XMVector3Cross( CoordSystem.vertex2, CoordSystem.vertex1 ) );		// Obliczamy wektor prostopad�y do normalnej i pierwszej osi.

	return CoordSystem;
}

/**@brief Tworzy macierz przekszta�caj�c� wierzcho�ki do uk�adu odniesienia emitera.*/
DirectX::XMMATRIX LightmapWorkerCPU::EmiterViewMatrix( Triangle4& emiter )
{
	// �rodek ci�ko�ci tr�jk�ta przyjujemy za �rodek emitera.
	XMVECTOR emiterCenter = XMVectorAdd( emiter.vertex1, emiter.vertex2 );
	emiterCenter = XMVectorAdd( emiterCenter, emiter.vertex3 );
	emiterCenter = XMVectorDivide( emiterCenter, XMVectorReplicate( 3.0f ) );

	// Wektor prostopad�y do p�aszczyzny, na kt�rej le�� tr�jk�ty.
	XMVECTOR edge12 = XMVectorSubtract( emiter.vertex2, emiter.vertex1 );
	XMVECTOR edge13 = XMVectorSubtract( emiter.vertex3, emiter.vertex1 );
	XMVECTOR normal = XMVector3Normalize( XMVector3Cross( edge12, edge13 ) );

	// Ustalamy dowolnie (byle ortogonalnie) kierunek "w g�r�".
	//XMVECTOR upDirection = XMVector3Normalize( XMVector3Cross( edge12, normal ) );
	XMVECTOR upDirection = XMVector3Orthogonal( normal );

	//XMVECTOR test = XMVector3Dot( normal, upDirection );

	// Tworzymy macierz widoku.
	normal = XMVectorSetW( normal, 0.0f );
	emiterCenter = XMVectorSetW( emiterCenter, 1.0f );
	upDirection = XMVectorSetW( upDirection, 0.0f );

	XMMATRIX viewMatrix = XMMatrixLookToRH( emiterCenter, normal, upDirection );
	return viewMatrix;
}

/**@brief Rasteryzuje podany tr�jk�t. Sprawdza czy jest bli�ej emitera od ostatnio
wpisanego tr�jk�ta na podstawie bufora g��boko�ci i je�eli tak, to zapisuje nowy tr�jk�t.

Rasteryzacja zrobiona mniej wi�cej na podstawie strony:
https://fgiesen.wordpress.com/2013/02/06/the-barycentric-conspirac/
Warto przeczyta� te� kilka temat�w pokrewnych. Na razie implementuj� wersj� niewydajn�.
Mo�e potem przerobi� na lepsz�. W ka�dym razie warto zauwa�y�, �e ten algorytm jest bardzo dobry
do zr�wnoleglania, mo�e da si� to jako� sprytnie wykorzysta�.

@param[in] triangle Tr�jk�t do rasteryzacji.
@param[in] depths Odleg�o�ci (g��boko�ci) kolejnych wierzcho�k�w tr�jk�ta od emitera. (Tu jest ma�e oszustwo,
zobaczymy jak b�dzie si� sprawdza�, odleg�o�ci s� liczone do �rodka emitera).
@param[in] chunkIdx Indeks MemoryChunka do wpisania do bufora indeks�w.
@param[in] triangleIdx Indeks tr�jk�ta, kt�ry trafi do bufora indeks�w.
@param[in] depthBuffer Bufor g��boko�ci.
@param[in] indexBuffer Bufor indeks�w.*/
void LightmapWorkerCPU::RasterizeTriangle( const Triangle4& triangle,
										   DirectX::XMFLOAT3* depths,
										   unsigned int chunkIdx,
										   unsigned int triangleIdx,
										   MemoryChunk& depthBuffer,
										   MemoryChunk& indexBuffer )
{
	XMFLOAT2A triangles[3];
	XMStoreFloat2A( &triangles[0], triangle.vertex1 );
	XMStoreFloat2A( &triangles[1], triangle.vertex2 );
	XMStoreFloat2A( &triangles[2], triangle.vertex3 );

	// Wyliczamy prostok�t ograniczaj�cy.
    int minX = min3( triangles[ 0 ].x, triangles[ 1 ].x, triangles[ 2 ].x);
    int minY = min3( triangles[ 0 ].y, triangles[ 1 ].y, triangles[ 2 ].y);
    int maxX = max3( triangles[ 0 ].x, triangles[ 1 ].x, triangles[ 2 ].x);
    int maxY = max3( triangles[ 0 ].y, triangles[ 1 ].y, triangles[ 2 ].y);

	// Iterujemy po wszystkich elementach prostok�ta ograniczaj�cego.
	// Sprawdzamy czy dany punkt nale�y do tr�jk�ta. Je�eli nale�y
	// zagl�damy do z bufora.
	XMINT2 point;
	for( point.y = minY; point.y <= maxY; point.y++ )
	{
		for( point.x = minX; point.x <= maxX; point.x++ )
		{
            float w0 = BarycentricCoords( triangles[1], triangles[2], point );
            float w1 = BarycentricCoords( triangles[2], triangles[0], point );
            float w2 = BarycentricCoords( triangles[0], triangles[1], point );

			if( w0 >= 0 && w1 >= 0 && w2 >= 0 )	// Warunek na to, �e punkt jest wewn�trz tr�jk�ta.
			{
				float sum = w0 + w1 + w2;
				w0 = w0 / sum;
				w1 = w1 / sum;
				w2 = w2 / sum;

				int index = point.y * m_depthResolution + point.x;
				//float pointInverseDepth = w0 / ((float*)depths)[ 0 ] + w1 / ((float*)depths)[ 1 ] + w2 / ((float*)depths)[ 2 ];
				//float pointDepth = 1.0f / pointInverseDepth;
				float pointDepth = ((float*)depths)[ 0 ] * w0 + ((float*)depths)[ 1 ] * w1 + ((float*)depths)[ 2 ] * w2;
				float& bufferDepth = depthBuffer.Get<float>( index );
				if( bufferDepth > pointDepth && pointDepth > 0.0f )
				{
					BufferIndexing& indicies = indexBuffer.Get<BufferIndexing>( index );
					indicies.first = chunkIdx;
					indicies.second = triangleIdx;
					bufferDepth = pointDepth;
				}
			}
		}
	}

}

/**@brief Wylicza rzut punktu na p�aszczyzn�.

@param[in] point Punkt wyra�ony jako wektor znormalizowany od �rodka hemisfery do rzutowanego punktu.
@param[in] coordsSystem Opisuje uk�ad wsp�rz�dnych w jakim ma by� wyra�ony zwracany punkt.
@return Funkcja zwaraca dwuwymiarowy wektor oznaczaj�cy punkt we wsp�rzednych w podanym
uk�adzie wsp�rz�dnych.*/
XMVECTOR LightmapWorkerCPU::ProjectPointToPlane( XMVECTOR point, const Triangle4& coordsSystem )
{
	XMVECTOR axisU = XMVector3Dot( coordsSystem.vertex2, point );
	XMVECTOR axisV = XMVector3Dot( coordsSystem.vertex3, point );
	XMVECTOR result = XMVectorMergeXY( axisU, axisV );

	return result;
}

/**@brief U�rednia trzy normalne tr�jk�ta.*/
XMVECTOR LightmapWorkerCPU::AverageNormal( const VertexFormat* triangle )
{
	XMVECTOR normal1 = XMLoadFloat3( &(*triangle).normal );		++triangle;
	XMVECTOR normal2 = XMLoadFloat3( &(*triangle).normal );		++triangle;
	XMVECTOR normal3 = XMLoadFloat3( &(*triangle).normal );

	return XMVectorDivide( XMVectorAdd( XMVectorAdd( normal1, normal2 ), normal3 ), XMVectorReplicate( 3.0f ) );
}

///**@brief Wylicza g��boko�� dla poszczeg�lnych wierzcho�k�w tr�jk�ta.*/
//DirectX::XMFLOAT3 LightmapWorkerCPU::ComputeDepths( const Triangle4& receiver, const Triangle4& emiter )
//{
//	XMVECTOR depth1 = XMVector3Length( XMVectorSubtract( receiver.vertex1, emiter.vertex1 ) );
//	XMVECTOR depth2 = XMVector3Length( XMVectorSubtract( receiver.vertex2, emiter.vertex2 ) );
//	XMVECTOR depth3 = XMVector3Length( XMVectorSubtract( receiver.vertex3, emiter.vertex3 ) );
//
//	XMFLOAT3 result;
//	XMStoreFloat( &result.x, depth1 );
//	XMStoreFloat( &result.y, depth2 );
//	XMStoreFloat( &result.z, depth3 );
//
//	return result;
//}

/**@brief Wylicza wsp�rz�dn� barycentryczn� punktu wzgl�dem kraw�dzie.
Wsp�rz�dne nie s� wyskalowane do jedynki. Nale�y je wyskalowa� kiedy si� wyliczy wszystkie.*/
float LightmapWorkerCPU::BarycentricCoords( DirectX::XMFLOAT2& vertex1, DirectX::XMFLOAT2& vertex2, DirectX::XMINT2& point )
{
	return (vertex2.x - vertex1.x) * ( point.y - vertex1.y ) - (vertex2.y - vertex1.y) * ( point.x - vertex1.x );
}

/**@brief Transformuje wierzcho�ki w przedziale [-1,1] do wsp�rzednych bufora g��boko�ci.*/
void LightmapWorkerCPU::HemisphereViewport( Triangle4& receiver )
{
	XMVECTOR depthResolution = XMVectorReplicate( (float)m_depthResolution / 2.0f );
	receiver.vertex1 = XMVectorMultiplyAdd( receiver.vertex1, depthResolution, depthResolution );
	receiver.vertex2 = XMVectorMultiplyAdd( receiver.vertex2, depthResolution, depthResolution );
	receiver.vertex3 = XMVectorMultiplyAdd( receiver.vertex3, depthResolution, depthResolution );
}

