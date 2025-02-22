/**
@file DX11PixelShader.cpp
@author nieznanysprawiciel
@copyright File is part of graphic engine SWEngine.
*/
#include "stdafx.h"

#include "DX11PixelShader.h"

#include "Common/MemoryLeaks.h"


//====================================================================================//
//			RTTR registration	
//====================================================================================//

RTTR_REGISTRATION
{
	rttr::registration::class_< DX11PixelShader >( "DX11PixelShader" );
}


//====================================================================================//
//			DX11PixelShader	
//====================================================================================//

/**@brief */
DX11PixelShader::DX11PixelShader( ID3D11PixelShader* shader )
{
	m_pixelShader = shader;
}

/**@brief */
DX11PixelShader::~DX11PixelShader()
{
	if( m_pixelShader )
		m_pixelShader->Release();
	m_pixelShader = nullptr;
}

/**@brief */
bool DX11PixelShader::ReloadFromFile()
{

	return false;
}

/**@brief */
bool DX11PixelShader::ReloadFromBinFile()
{

	return false;
}

/**@brief */
void DX11PixelShader::SaveShaderBinFile( const std::wstring& fileName )
{
	assert( false );

}

/**@bref Tworzy obiekt DX11PixelShader na podstawie pliku.

W przypadku błędów kompilacji w trybie debug są one przekierowane do okna Output.

Na razie obsługuje tylko nieskompilowane pliki.
@param[in] fileName Nazwa pliku, z którego zostanie wczytany shader.
@param[in] shaderName Nazwa funkcji, która jest punktem poczatkowym wykonania shadera.
@param[in] shaderModel Łańcuch znaków opisujący shader model.
@return Zwaraca wskaźnik na DX11VertexShader lub nullptr w przypadku błędów wczytywania bądź kompilacji.
*/
DX11PixelShader* DX11PixelShader::CreateFromFile( const std::wstring& fileName, const std::string& shaderName, const char* shaderModel )
{
	HRESULT result;
	ID3DBlob* compiled_shader;
	ID3D11PixelShader* pixel_shader;
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
						   0, 0, &compiled_shader,
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
	result = device->CreatePixelShader( compiled_shader->GetBufferPointer(),
										compiled_shader->GetBufferSize(),
										nullptr, &pixel_shader );

	if( FAILED( result ) )
	{
		compiled_shader->Release();
		return nullptr;
	}

	// Tworzymy obiekt nadający się do użycia w silniku i zwracamy wskaźnik na niego
	DX11PixelShader* new_shader = new DX11PixelShader( pixel_shader );
	return new_shader;
}

DX11PixelShader* DX11PixelShader::CreateFromBinFile( const std::wstring& fileName, const std::string& shaderName, const char* shaderModel)
{
	return nullptr;
}

