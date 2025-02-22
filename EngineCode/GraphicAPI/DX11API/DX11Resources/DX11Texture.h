#pragma once

#include "DX11Initializer/DX11APIObjects.h"
#include "GraphicAPI/MeshResources.h"


/**@brief Implementacja tekstury w DirectX 11.
@ingroup DX11API*/
class DX11Texture	:	public TextureObject, protected DX11APIObjects
{
	RTTR_ENABLE( TextureObject )
private:
	ComPtr< ID3D11ShaderResourceView >		m_textureView;
	ComPtr< ID3D11Texture2D >				m_texture;

	TextureInfo								m_descriptor;
protected:
	~DX11Texture();
public:
	explicit								DX11Texture				( TextureInfo&& texInfo, ID3D11Texture2D* tex, ID3D11ShaderResourceView* texView );
	explicit								DX11Texture				( TextureInfo&& texInfo, ComPtr< ID3D11Texture2D > tex, ComPtr< ID3D11ShaderResourceView > texView );

	virtual MemoryChunk						CopyData				() const override;
	virtual const TextureInfo&				GetDescriptor			() const override;

	virtual const filesystem::Path&			GetFilePath				() const override;

	static DX11Texture*						CreateFromMemory		( const MemoryChunk& texData, TextureInfo&& texInfo );

	inline ID3D11ShaderResourceView*		Get			()			{ return m_textureView.Get(); }
	inline ID3D11Texture2D*					GetTex		()			{ return m_texture.Get(); }
	static D3D11_TEXTURE2D_DESC				FillDesc	( const TextureInfo& texInfo );


private:
	void		Construct	();
};

