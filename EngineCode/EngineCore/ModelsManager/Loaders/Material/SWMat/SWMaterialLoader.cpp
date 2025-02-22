#include "EngineCore/stdafx.h"
#include "SWMaterialLoader.h"
#include "SWMaterialLoader.inl"

#include "Common/Converters.h"
#include "Common/Serialization/SW/Serialization.h"
#include "Common/Serialization/Deserializer.h"

#include "EngineCore/ModelsManager/AssetsManager.h"
#include "EngineCore/ModelsManager/Assets/Materials/ShadingModelData.h"
#include "EngineCore/ModelsManager/Assets/Materials/PhongMaterialData.h"



SWMaterialLoader::SWMaterialLoader( AssetsManager* modelsManager )
	:	m_modelsManager( modelsManager )
{
	m_versionMajor = 1;
	m_versionMinor = 0;
}

SWMaterialLoader::~SWMaterialLoader()
{}



const std::string		SWMaterialLoader::STRINGS_1_0::FILE_HEADER_STRING		= "SWMaterial";
const std::string		SWMaterialLoader::STRINGS_1_0::VERSION_MAJOR_STRING		= "VersionMajor";
const std::string		SWMaterialLoader::STRINGS_1_0::VERSION_MINOR_STRING		= "VersionMinor";

const std::string		SWMaterialLoader::STRINGS_1_0::FILE_PATH_STRING			= "FilePath";
const std::string		SWMaterialLoader::STRINGS_1_0::SHADER_ENTRY_STRING		= "ShaderEntry";

const std::string		SWMaterialLoader::STRINGS_1_0::VERTEX_SHADER_STRING		= "VertexShader";
const std::string		SWMaterialLoader::STRINGS_1_0::PIXEL_SHADER_STRING		= "PixelShader";
const std::string		SWMaterialLoader::STRINGS_1_0::GEOMETRY_SHADER_STRING	= "GeometryShader";
const std::string		SWMaterialLoader::STRINGS_1_0::CONTROL_SHADER_STRING	= "TesselationControlShader";
const std::string		SWMaterialLoader::STRINGS_1_0::EVALUATION_SHADER_STRING = "TesselationEvaluationShader";

const std::string		SWMaterialLoader::STRINGS_1_0::TEXTURES_ARRAY_STRING	= "Textures";

const std::string		SWMaterialLoader::STRINGS_1_0::ADD_BUFFERS_ARRAY_STRING = "AdditionalBuffers";
const std::string		SWMaterialLoader::STRINGS_1_0::BUFFER_SIZE_STRING		= "BufferSize";
const std::string		SWMaterialLoader::STRINGS_1_0::SHADING_DATA_STRING		= "ShadingModel";
const std::string		SWMaterialLoader::STRINGS_1_0::SHADING_MODEL_WRAPPER_TYPE_STRING		= "WrapperType";




// ================================ //
//

Nullable< MaterialInitData >		SWMaterialLoader::LoadMaterial	( const filesystem::Path& fileName )
{
	IDeserializer		deser( std::make_unique< EngineSerializationContext >() );

	if( !deser.LoadFromFile( fileName.String(), ParsingMode::ParseInsitu ) )
		return Nullable< MaterialInitData >( deser.GetError() );

	if( deser.EnterObject( STRINGS_1_0::FILE_HEADER_STRING ) )
	{
		uint32 versionMajor = deser.GetAttribute( STRINGS_1_0::VERSION_MAJOR_STRING, 0 );
		uint32 versionMinor = deser.GetAttribute( STRINGS_1_0::VERSION_MINOR_STRING, 0 );

		if( m_versionMajor >= versionMajor && m_versionMinor >= versionMinor && versionMajor >= 1 )
		{
			if( versionMajor == 1 )
				return LoadMaterial_Version1( &deser );
		}
		else
		{
			std::string errorString =  "File version " + Convert::ToString( versionMajor ) + "." + Convert::ToString( versionMinor ) + " not supported. Maximal version " + Convert::ToString( m_versionMajor ) + "." + Convert::ToString( m_versionMinor );
			return Nullable< MaterialInitData >( std::move( errorString ) );
		}


		deser.Exit();
	}

	return Nullable< MaterialInitData >( "Invalid .swmat file. Header not found." );
}

// ================================ //
//
bool								SWMaterialLoader::CanLoad		( const filesystem::Path& fileName )
{
	if( fileName.GetExtension() == ".swmat" )
		return true;
	return false;
}

// ================================ //
//
void								SWMaterialLoader::SaveMaterial	( const filesystem::Path& fileName, MaterialAsset* mat )
{
	ISerializer ser( std::make_unique< EngineSerializationContext >() );

	ser.EnterObject( STRINGS_1_0::FILE_HEADER_STRING );

		ser.SetAttribute( STRINGS_1_0::VERSION_MAJOR_STRING, m_versionMajor );
		ser.SetAttribute( STRINGS_1_0::VERSION_MINOR_STRING, m_versionMinor );

		ser.EnterObject( rttr::type::get< MaterialAsset >().get_name() );

			// Shaders

			IShader* shader = mat->GetVertexShader().Ptr();
			ser.EnterObject( STRINGS_1_0::VERTEX_SHADER_STRING );
			if( shader )
			{
				ser.SetAttribute( STRINGS_1_0::FILE_PATH_STRING, Convert::ToString( shader->GetShaderFile() ) );
				ser.SetAttribute( STRINGS_1_0::SHADER_ENTRY_STRING, shader->GetShaderEntry() );
			}
			ser.Exit();

			shader = mat->GetPixelShader().Ptr();
			ser.EnterObject( STRINGS_1_0::PIXEL_SHADER_STRING );
			if( shader )
			{
				ser.SetAttribute( STRINGS_1_0::FILE_PATH_STRING, Convert::ToString( shader->GetShaderFile() ) );
				ser.SetAttribute( STRINGS_1_0::SHADER_ENTRY_STRING, shader->GetShaderEntry() );
			}
			ser.Exit();

			shader = mat->GetGeometryShader().Ptr();
			ser.EnterObject( STRINGS_1_0::GEOMETRY_SHADER_STRING );
			if( shader )
			{
				ser.SetAttribute( STRINGS_1_0::FILE_PATH_STRING, Convert::ToString( shader->GetShaderFile() ) );
				ser.SetAttribute( STRINGS_1_0::SHADER_ENTRY_STRING, shader->GetShaderEntry() );
			}
			ser.Exit();

			shader = mat->GetTessControlShader().Ptr();
			ser.EnterObject( STRINGS_1_0::CONTROL_SHADER_STRING );
			if( shader )
			{
				ser.SetAttribute( STRINGS_1_0::FILE_PATH_STRING, Convert::ToString( shader->GetShaderFile() ) );
				ser.SetAttribute( STRINGS_1_0::SHADER_ENTRY_STRING, shader->GetShaderEntry() );
			}
			ser.Exit();

			shader = mat->GetTessEvaluationShader().Ptr();
			ser.EnterObject( STRINGS_1_0::EVALUATION_SHADER_STRING );
			if( shader )
			{
				ser.SetAttribute( STRINGS_1_0::FILE_PATH_STRING, Convert::ToString( shader->GetShaderFile() ) );
				ser.SetAttribute( STRINGS_1_0::SHADER_ENTRY_STRING, shader->GetShaderEntry() );
			}
			ser.Exit();

			// Textures
			ser.EnterArray( STRINGS_1_0::TEXTURES_ARRAY_STRING );

			for( int i = 0; i < MAX_MATERIAL_TEXTURES; ++i )
			{
				auto tex = mat->GetTexture( i ).Ptr();

				ser.EnterObject( rttr::type::get< TextureObject >().get_name() );

				if( tex )
					ser.SetAttribute( STRINGS_1_0::FILE_PATH_STRING, tex->GetFilePath().String() );

				ser.Exit();
			}

			ser.Exit();	// Textures

			
			// Shading model data
			auto shadingData = mat->GetDescriptor().ShadingData.get();

			TypeID shadingModelType = shadingData->GetShadingModelType();
			TypeID shadingModelPtrType = shadingData->GetShadingModelPtrType();

			rttr::variant shadingDataPtr( (void*)shadingData->GetShadingModelData() );
			shadingDataPtr.unsafe_convert_void( shadingModelPtrType );


			ser.EnterObject( STRINGS_1_0::SHADING_DATA_STRING );

				ser.SetAttribute( STRINGS_1_0::SHADING_MODEL_WRAPPER_TYPE_STRING, shadingData->GetTypeName() );
				ser.SetAttribute( STRINGS_1_0::BUFFER_SIZE_STRING, shadingData->GetShadingModelSize() );
				Serialization::DefaultSerializeImpl( &ser, shadingDataPtr, shadingModelType );

			ser.Exit();		// SHADING_DATA_STRING

			// Additional buffers
			TypeID descriptorType = TypeID::get< MaterialInfo >();
			auto additionalBuffersProperty = descriptorType.get_property( STRINGS_1_0::ADD_BUFFERS_ARRAY_STRING.c_str() );

			Serialization::SerializeArrayTypes( &ser, mat->GetDescriptor(), additionalBuffersProperty );


		ser.Exit();	// type name

	ser.Exit(); // SWMaterial

	ser.SaveFile( fileName.String(), WritingMode::Readable );
}

// ================================ //
// @todo Maybe we should tell AssetsManager to release resources, if something went wrong.
Nullable< MaterialInitData >		SWMaterialLoader::LoadMaterial_Version1	( IDeserializer* deser )
{
	Nullable< MaterialInitData >	data;
	data.IsValid = true;

	if( deser->EnterObject( rttr::type::get< MaterialAsset >().get_name() ) )
	{
		data = LoadShaders( deser, data );
		data = LoadTextures( deser, data );
		data = LoadShadingData( deser, data );
		data = LoadAdditionalBuffers( deser, data );

		deser->Exit();
	}

	return std::move( data );
}


Nullable< MaterialInitData >		SWMaterialLoader::LoadShaders		( IDeserializer* deser, Nullable< MaterialInitData >& init )
{
	ReturnIfInvalid( init );

	auto vertexShader = LoadShader< VertexShader >( deser, STRINGS_1_0::VERTEX_SHADER_STRING );
	auto pixelShader = LoadShader< PixelShader >( deser, STRINGS_1_0::PIXEL_SHADER_STRING );

	// Vertex shader and pixel shader must be set.
	ReturnIfInvalid( vertexShader );
	ReturnIfInvalid( pixelShader );

	init.Value.VertexShader = std::move( vertexShader.Value );
	init.Value.PixelShader = std::move( pixelShader.Value );

	auto geometryShader = LoadShader< GeometryShader >( deser, STRINGS_1_0::GEOMETRY_SHADER_STRING );
	auto evaluationShader = LoadShader< EvaluationShader >( deser, STRINGS_1_0::EVALUATION_SHADER_STRING );
	auto controlShader = LoadShader< ControlShader >( deser, STRINGS_1_0::CONTROL_SHADER_STRING );

	// Note: We don't check if shader was created. Nullptrs are acceptable value.
	init.Value.GeometryShader = std::move( geometryShader.Value );
	init.Value.TesselationEvaluationShader = std::move( evaluationShader.Value );
	init.Value.TesselationControlShader = std::move( controlShader.Value );

	return std::move( init );
}

Nullable< MaterialInitData >		SWMaterialLoader::LoadTextures		( IDeserializer* deser, Nullable< MaterialInitData >& init )
{
	ReturnIfInvalid( init );

	// Textures
	if( deser->EnterArray( STRINGS_1_0::TEXTURES_ARRAY_STRING ) )
	{
		if( deser->FirstElement() )
		{
			int texIdx = 0;
			do
			{
				assert( deser->GetName() == TypeID::get< TextureObject >().get_name() );
					
				std::string fileName = deser->GetAttribute( STRINGS_1_0::FILE_PATH_STRING, "" );
				if( fileName != "" )
				{
					auto tex = m_modelsManager->LoadTexture( Convert::FromString< std::wstring >( fileName, L"" ) );
					init.Value.Textures[ texIdx ] = tex;
				}
				
				texIdx++;
			} while( deser->NextElement() && texIdx < MAX_MATERIAL_TEXTURES );

		}
		deser->Exit();
	}

	return std::move( init );
}

Nullable< MaterialInitData >		SWMaterialLoader::LoadShadingData	( IDeserializer* deser, Nullable< MaterialInitData >& init )
{
	ReturnIfInvalid( init );
	auto& initData = init.Value;


	// Shading model data
	if( deser->EnterObject( STRINGS_1_0::SHADING_DATA_STRING ) )
	{
		uint32		bufferSize = deser->GetAttribute( STRINGS_1_0::BUFFER_SIZE_STRING, (uint32)0 );
		const char*	wrapperName = deser->GetAttribute( STRINGS_1_0::SHADING_MODEL_WRAPPER_TYPE_STRING, nullptr );

		if( !wrapperName )
			return Nullable< MaterialInitData >( "File doesn't contain WrapperType field." );

		TypeID wrapper = TypeID::get_by_name( wrapperName );
		if( !wrapper.is_valid() )
			return Nullable< MaterialInitData >( "WrapperType is not registered." );
			
		auto constructor = wrapper.get_constructor();
		if( !constructor.is_valid() )
			return Nullable< MaterialInitData >( "WrapperType constructor is not registered." );

		rttr::variant object = wrapper.create();
		if( !object.is_valid() )
			return Nullable< MaterialInitData >( "WrapperType could not be created." );

		if( !object.can_convert( TypeID::get< ShadingModelBase* >() ) )
			return Nullable< MaterialInitData >( "Can't convert MaterialData to ShadingModelBase*. Make sure, constructor was declared with AsRawPtr policy." );

		initData.ShadingData = UPtr< ShadingModelBase >( object.get_value< ShadingModelBase* >() );

		if( initData.ShadingData->GetShadingModelSize() != bufferSize )
			return Nullable< MaterialInitData >( "Declared buffer size is other then real buffer size." );


		rttr::variant dataObject = (void*)initData.ShadingData->GetShadingModelData();
		dataObject.unsafe_convert_void( initData.ShadingData->GetShadingModelPtrType() );


		if( deser->EnterObject( initData.ShadingData->GetShadingModelTypeName() ) )
		{
			Serialization::DefaultDeserializeImpl( deser, dataObject, initData.ShadingData->GetShadingModelType() );

			deser->Exit();
		}
		else
			return Nullable< MaterialInitData >( initData.ShadingData->GetShadingModelTypeName() + " not found in file." );

		deser->Exit();
	}

	return std::move( init );
}

Nullable< MaterialInitData >		SWMaterialLoader::LoadAdditionalBuffers	( IDeserializer* deser, Nullable< MaterialInitData >& init )
{
	ReturnIfInvalid( init );
	auto& initData = init.Value;

	std::vector< AdditionalBufferInfo >& addBuffers = init.Value.AdditionalBuffers;

	if( deser->EnterArray( STRINGS_1_0::ADD_BUFFERS_ARRAY_STRING ) )
	{
		auto arraySize = deser->GetAttribute( "ArraySize", 0 );
		if( arraySize != 0 )
			addBuffers.reserve( arraySize );

		if( deser->FirstElement() )
		{
			do
			{
				AdditionalBufferInfo newBuffer;
				addBuffers.push_back( newBuffer );
				auto newBuffRealPtr = &addBuffers[ addBuffers.size() - 1 ];

				rttr::variant buffVariant = newBuffRealPtr;

				Serialization::DefaultDeserializeImpl( deser, buffVariant, TypeID::get< AdditionalBufferInfo >() );

			} while( deser->NextElement() );
		}

		deser->Exit();
	}
	// else there's no buffers probably. It's not an error.

	return std::move( init );
}



// ================================ //
//
SWMaterialLoader::ShaderLoadInfo	SWMaterialLoader::DeserializeShader	( IDeserializer* deser, const std::string& shaderNameString )
{
	const char* shaderFile = nullptr;
	const char* shaderEntry = nullptr;

	if( deser->EnterObject( shaderNameString ) )
	{
		shaderEntry = deser->GetAttribute( STRINGS_1_0::SHADER_ENTRY_STRING, (const char*)nullptr );
		shaderFile = deser->GetAttribute( STRINGS_1_0::FILE_PATH_STRING, (const char*)nullptr );

		deser->Exit();
	}

	return ShaderLoadInfo( shaderFile, shaderEntry );
}

