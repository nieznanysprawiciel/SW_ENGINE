/**
@file DX11ComputeShader.cpp
@author nieznanysprawiciel
@copyright File is part of graphic engine SWEngine.
*/
#include "stdafx.h"


#include "DX11ComputeShader.h"

#include "Common/MemoryLeaks.h"


RTTR_REGISTRATION
{
	rttr::registration::class_< DX11ComputeShader >( "DX11ComputeShader" );
}


DX11ComputeShader::DX11ComputeShader( ID3D11ComputeShader* shader )
{
	m_computeShader = shader;
}

DX11ComputeShader::~DX11ComputeShader()
{
	if( m_computeShader )
		m_computeShader->Release();
	m_computeShader = nullptr;
}

bool DX11ComputeShader::ReloadFromFile()
{
	return false;
}

bool DX11ComputeShader::ReloadFromBinFile()
{
	return false;
}

void DX11ComputeShader::SaveShaderBinFile( const std::wstring& fileName )
{

}

DX11ComputeShader* DX11ComputeShader::CreateFromFile( const std::wstring& fileName, const std::string& shaderName, const char* shaderModel )
{
	HRESULT result;
	ID3DBlob* compiledShader;
	ID3D11ComputeShader* computeShader;
	// Troszkę zamieszania, ale w trybie debug warto wiedzieć co jest nie tak przy kompilacji shadera
#ifdef _DEBUG
	ID3D10Blob* errorBlob = nullptr;	// Tu znajdzie się komunikat o błędzie
#endif

	// Kompilujemy shader znaleziony w pliku
	D3DX11CompileFromFile( fileName.c_str(), 0, 0, shaderName.c_str(), shaderModel,
						   0, 0, 0, &compiledShader,
#ifdef _DEBUG
						   &errorBlob,	// Funkcja wsadzi informacje o błędzie w to miejsce
#else
						   0,	// W trybie Release nie chcemy dostawać błędów
#endif
						   &result );

	if( FAILED( result ) )
	{
#ifdef _DEBUG
		if( errorBlob )
		{
			// Błąd zostanie wypisany na standardowe wyjście
			OutputDebugStringA( (char*)errorBlob->GetBufferPointer() );
			errorBlob->Release();
		}
#endif
		return nullptr;
	}

	// Tworzymy obiekt shadera na podstawie tego co sie skompilowało
	result = device->CreateComputeShader( compiledShader->GetBufferPointer(),
										compiledShader->GetBufferSize(),
										nullptr, &computeShader );

	if( FAILED( result ) )
	{
		compiledShader->Release();
		return nullptr;
	}

	// Tworzymy obiekt nadający się do użycia w silniku i zwracamy wskaźnik na niego
	DX11ComputeShader* newShader = new DX11ComputeShader( computeShader );
	return newShader;
}

DX11ComputeShader* DX11ComputeShader::CreateFromBinFile( const std::wstring& file_name )
{

	return nullptr;
}


