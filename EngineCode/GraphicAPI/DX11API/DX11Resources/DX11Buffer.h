#pragma once
/**
@file DX11Buffer.h
@author nieznanysprawiciel
@copyright Plik jest częścią silnika graficznego SWEngine.
*/


#include "GraphicAPI/MeshResources.h"
#include "DX11Initializer/DX11APIObjects.h"

/**@brief Implementacja bufora w DirectX11.
@ingroup DX11API*/
class DX11Buffer	:	public BufferObject, protected DX11APIObjects
{
	RTTR_ENABLE( BufferObject );
private:
	ID3D11Buffer*				m_buffer;
	BufferInfo					m_descriptor;
protected:
	~DX11Buffer();
public:
	DX11Buffer( const std::wstring& name, const BufferInfo& descriptor, ID3D11Buffer* buff );

	inline ID3D11Buffer*		Get() { return m_buffer; }

	static DX11Buffer*			CreateFromMemory( const std::wstring& name, const uint8* data, const BufferInfo& bufferInfo );

	virtual MemoryChunk			CopyData		() override;
	virtual const BufferInfo&	GetDescriptor	() const	{ return m_descriptor; }
};

