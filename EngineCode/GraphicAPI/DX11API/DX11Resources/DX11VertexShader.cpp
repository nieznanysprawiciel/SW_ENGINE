/**
@file DX11VertexShader.cpp
@author nieznanysprawiciel
@copyright File is part of graphic engine SWEngine.
*/
#include "stdafx.h"


#include "DX11VertexShader.h"

#include "DX11InputLayoutDescriptor.h"
#include "DX11InputLayout.h"

#include "Common/MemoryLeaks.h"


RTTR_REGISTRATION
{
	rttr::registration::class_< DX11VertexShader >( "DX11VertexShader" );
}

DX11VertexShader::DX11VertexShader( ID3D11VertexShader* shader )
{
	m_vertexShader = shader;
}

DX11VertexShader::~DX11VertexShader()
{
	if( m_vertexShader )
		m_vertexShader->Release();
	m_vertexShader = nullptr;
}


bool DX11VertexShader::ReloadFromFile()
{

	return false;
}

bool DX11VertexShader::ReloadFromBinFile()
{

	return false;
}

void DX11VertexShader::SaveShaderBinFile( const std::wstring& file_name )
{


}

/**@brief Tworzy obiekt DX11VertexShader na podstawie pliku.

W przypadku błędów kompilacji w trybie debug są one przekierowane do okna Output.

Na razie obsługuje tylko nieskompilowane pliki.
@param[in] fileName Nazwa pliku, z którego zostanie wczytany shader
@param[in] shaderName Nazwa funkcji, która jest punktem poczatkowym wykonania shadera
@param[in] shaderModel Łańcuch znaków opisujący shader model.
@return Zwraca wskaźnik na obiekt DX11VertexShader lub nullptr w przypadku niepowodzenia.*/
DX11VertexShader* DX11VertexShader::CreateFromFile( const std::wstring& fileName, const std::string& shaderName, const char* shaderModel )
{
	HRESULT result;
	ID3DBlob* compiledShader;
	ID3D11VertexShader* vertexShader;
	// Troszkę zamieszania, ale w trybie debug warto wiedzieć co jest nie tak przy kompilacji shadera
#ifdef _DEBUG
	ID3D10Blob* error_blob = nullptr;	// Tu znajdzie się komunikat o błędzie
#endif

	// Kompilujemy shader znaleziony w pliku
	D3DX11CompileFromFile( fileName.c_str(), 0, 0, shaderName.c_str(), shaderModel,
#ifdef _DEBUG						   
						   D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION,
#else
						   0,
#endif
						   0, 0, &compiledShader,
#ifdef _DEBUG
						   &error_blob,	// Funkcja wsadzi informacje o błędzie w to miejsce
#else
						   0,	// W trybie Release nie chcemy dostawać błędów
#endif
						   &result );

	if( FAILED( result ) )
	{
#ifdef _DEBUG
		if( error_blob )
		{
			// Błąd zostanie wypisany na standardowe wyjście
			OutputDebugStringA( (char*)error_blob->GetBufferPointer() );
			error_blob->Release();
		}
#endif
		return nullptr;
	}

	// Tworzymy obiekt shadera na podstawie tego co sie skompilowało
	result = device->CreateVertexShader( compiledShader->GetBufferPointer(),
										 compiledShader->GetBufferSize(),
										 nullptr, &vertexShader );

	if( FAILED( result ) )
	{
		compiledShader->Release();
		return nullptr;
	}

	// Tworzymy obiekt nadający się do użycia w silniku i zwracamy wskaźnik na niego
	DX11VertexShader* newShader = new DX11VertexShader( vertexShader );
	return newShader;
}

/**@brief Tworzy obiekt DX11VertexShader oraz DX11InputLayout na podstawie pliku. Zwraca również layout dla podanej struktury wierzchołka.
Nie należy używać tej funkcji, jeżeli layout nie jest rzeczywiście potrzebny. Trzeba pamietać o zwolnieniu
go, kiedy przestanie być potrzebny.

W przypadku błędów kompilacji w trybie debug są one przekierowane do okna Output.

Na razie obsługuje tylko nieskompilowane pliki.
@param[in] fileName Nazwa pliku, z którego zostanie wczytany shader
@param[in] shaderName Nazwa funkcji, która jest punktem poczatkowym wykonania shadera
@param[out] layout W zmiennej umieszczany jest wskaźnik na layout wierzchołka. Należy pamiętać o zwolnieniu go kiedy będzie niepotrzebny.
@param[in] layoutDesc Deskryptor opisujacy tworzony layout.
@param[in] shaderModel Łańcuch znaków opisujący shader model.
@return Zwraca wskaźnik na obiekt VertexShader lub nullptr w przypadku niepowodzenia.*/
DX11VertexShader* DX11VertexShader::CreateFromFile		( const std::wstring& fileName,
															const std::string& shaderName,
															ShaderInputLayout** layout,
															InputLayoutDescriptor* layoutDesc,
															const char* shaderModel/* = "vs_4_0" */)
{
	HRESULT result;
	ID3DBlob* compiledShader;
	ID3D11VertexShader* vertexShader;
	// Troszkę zamieszania, ale w trybie debug warto wiedzieć co jest nie tak przy kompilacji shadera
#ifdef _DEBUG
	ID3D10Blob* error_blob = nullptr;	// Tu znajdzie się komunikat o błędzie
#endif

	// Kompilujemy shader znaleziony w pliku
	D3DX11CompileFromFile( fileName.c_str( ), 0, 0, shaderName.c_str( ), shaderModel,
						   0, 0, 0, &compiledShader,
#ifdef _DEBUG
						   &error_blob,	// Funkcja wsadzi informacje o błędzie w to miejsce
#else
						   0,	// W trybie Release nie chcemy dostawać błędów
#endif
						   &result );

	if ( FAILED( result ) )
	{
#ifdef _DEBUG
		if ( error_blob )
		{
			// Błąd zostanie wypisany na standardowe wyjście
			OutputDebugStringA( (char*)error_blob->GetBufferPointer( ) );
			error_blob->Release( );
		}
#endif
		layout = nullptr;
		return nullptr;
	}

	// Tworzymy obiekt shadera na podstawie tego co sie skompilowało
	result = device->CreateVertexShader( compiledShader->GetBufferPointer( ),
										 compiledShader->GetBufferSize( ),
										 nullptr, &vertexShader );

	if ( FAILED( result ) )
	{
		compiledShader->Release();
		layout = nullptr;
		return nullptr;
	}

	// Tworzymy layout
	assert( layoutDesc );		///@todo Lepiej obsłużyć ten błąd.
	D3D11_INPUT_ELEMENT_DESC* layoutDescPtr = ((DX11InputLayoutDescriptor*)layoutDesc)->GetDescriptorPtr();
	Size layoutNumElements = ((DX11InputLayoutDescriptor*)layoutDesc)->GetNumElements();
	ID3D11InputLayout* DX11layoutInterface = nullptr;

	result = device->CreateInputLayout( layoutDescPtr, (UINT)layoutNumElements, compiledShader->GetBufferPointer(),
												compiledShader->GetBufferSize(), &DX11layoutInterface );
	compiledShader->Release( );
	if ( FAILED( result ) )
	{
		layout = nullptr;
		return nullptr;
	}

	// Tworzymy obiekt nadający się do użycia w silniku i zwracamy wskaźnik na niego
	DX11VertexShader* newShader = new DX11VertexShader( vertexShader );
	*layout = new DX11InputLayout( DX11layoutInterface );
	return newShader;
}


DX11VertexShader* DX11VertexShader::CreateFromBinFile( const std::wstring& fileName, const std::string& shaderName, const char* shaderModel)
{

	return nullptr;
}

