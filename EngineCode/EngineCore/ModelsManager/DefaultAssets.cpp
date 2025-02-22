#include "EngineCore/stdafx.h"
/**
@file DefaultAssets.cpp
@author nieznanysprawiciel
@copyright Plik jest cz�ci� silnika graficznego SWEngine.
*/

#include "DefaultAssets.h"
#include "GraphicAPI/ResourcesFactory.h"

#include "Common/MemoryLeaks.h"




// RenderTargety
const wchar_t*	DefaultAssets::SCREEN_RENDERTARGET_STRING = L"::Screen render target";
const wchar_t*	DefaultAssets::EDITOR_RENDERTARGET_STRING = L"::Editor render target";

// Renderer state
const wchar_t*	DefaultAssets::DEFAULT_BLENDING_STATE_STRING		= L"::Default";
const wchar_t*	DefaultAssets::DEFAULT_RASTERIZER_STATE_STRING		= L"::Default";
const wchar_t*	DefaultAssets::DEFAULT_DEPTH_STATE_STRING			= L"::Default";

// Nazwy dla domy�lnych shader�w i materia�u
const wchar_t*	DefaultAssets::DEFAULT_MATERIAL_STRING				= L"::default_material";						///<Neutralny materia�.
const wchar_t*	DefaultAssets::DEFAULT_VERTEX_SHADER_STRING			= L"shaders/default_shaders.fx";				///<Shader bez obs�ugi tekstur.
const wchar_t*	DefaultAssets::DEFAULT_PIXEL_SHADER_STRING			= L"shaders/default_shaders.fx";				///<Shader bez obs�ugi tekstur.
const wchar_t*	DefaultAssets::DEFAULT_TEX_DIFFUSE_PIXEL_SHADER_PATH	= L"shaders\\tex_diffuse_shader.fx";		///<Shader z obs�ug� tesktury diffuse.
const wchar_t*	DefaultAssets::DEFAULT_COORD_COLOR_PIXEL_SHADER_PATH	= L"shaders/LightmapGen.fx";				///<Shader do generowania lightmap.
const wchar_t*	DefaultAssets::DEFAULT_COORD_COLOR_VERTEX_SHADER_PATH	= L"shaders/LightmapGen.fx";				///<Shader do generowania lightmap.
const wchar_t*	DefaultAssets::DEFAULT_LIGHTMAP_PIXEL_SHADER_PATH		= L"shaders/MaterialLightmap.fx";			///<Shader u�ywaj�cy materia�u i lightmapy.

// Domy�lne nazwy funkcji w vertex i pixel shaderze
const char*	DefaultAssets::DEFAULT_VERTEX_SHADER_ENTRY = "vertex_shader";		///<Domy�lna nazwa funkcji, od kt�rej zaczyna si� wykonanie vertex shadera
const char*	DefaultAssets::DEFAULT_PIXEL_SHADER_ENTRY = "pixel_shader";		///<Domy�lna nazwa funkcji, od kt�rej zaczyna si� wykonanie pixel shadera


// Semantyki dla vertex shadera
const char*	DefaultAssets::SEMANTIC_POSITION		= "POSITION";
const char*	DefaultAssets::SEMANTIC_NORMAL			= "NORMAL";
const char*	DefaultAssets::SEMANTIC_TEXCOORD		= "TEXCOORD";
const char*	DefaultAssets::SEMANTIC_COLOR			= "COLOR";


InputLayoutDescriptor* DefaultAssets::LAYOUT_POSITION_NORMAL_COORD = nullptr;
InputLayoutDescriptor* DefaultAssets::LAYOUT_POSITION_COORD = nullptr;
InputLayoutDescriptor* DefaultAssets::LAYOUT_POSITION_COLOR = nullptr;
InputLayoutDescriptor* DefaultAssets::LAYOUT_COORD_COLOR = nullptr;


// ================================ //
//
void DefaultAssets::Init()
{
	// 
	LAYOUT_POSITION_NORMAL_COORD = ResourcesFactory::CreateInputLayoutDescriptor( L"::PositionNormalCoord" );
	LAYOUT_POSITION_NORMAL_COORD->AddRow( SEMANTIC_POSITION, ResourceFormat::RESOURCE_FORMAT_R32G32B32_FLOAT, 0, 0, false, 0 );
	LAYOUT_POSITION_NORMAL_COORD->AddRow( SEMANTIC_NORMAL, ResourceFormat::RESOURCE_FORMAT_R32G32B32_FLOAT, 0, 12, false, 0 );
	LAYOUT_POSITION_NORMAL_COORD->AddRow( SEMANTIC_TEXCOORD, ResourceFormat::RESOURCE_FORMAT_R32G32_FLOAT, 0, 24, false, 0 );

	//
	LAYOUT_POSITION_COORD = ResourcesFactory::CreateInputLayoutDescriptor( L"::PositionCoord" );
	LAYOUT_POSITION_COORD->AddRow( SEMANTIC_POSITION, ResourceFormat::RESOURCE_FORMAT_R32G32B32_FLOAT, 0, 0, false, 0 );
	LAYOUT_POSITION_COORD->AddRow( SEMANTIC_TEXCOORD, ResourceFormat::RESOURCE_FORMAT_R32G32_FLOAT, 0, 12, false, 0 );

	//
	LAYOUT_POSITION_COLOR = ResourcesFactory::CreateInputLayoutDescriptor( L"::PositionColor" );
	LAYOUT_POSITION_COLOR->AddRow( SEMANTIC_POSITION, ResourceFormat::RESOURCE_FORMAT_R32G32B32_FLOAT, 0, 0, false, 0 );
	LAYOUT_POSITION_COLOR->AddRow( SEMANTIC_COLOR, ResourceFormat::RESOURCE_FORMAT_R32G32B32_FLOAT, 0, 12, false, 0 );

	//
	LAYOUT_COORD_COLOR = ResourcesFactory::CreateInputLayoutDescriptor( L"::CoordColor" );
	LAYOUT_COORD_COLOR->AddRow( SEMANTIC_TEXCOORD, ResourceFormat::RESOURCE_FORMAT_R32G32_FLOAT, 0, 0, false, 0 );
	LAYOUT_COORD_COLOR->AddRow( SEMANTIC_COLOR, ResourceFormat::RESOURCE_FORMAT_R32G32B32_FLOAT, 0, 8, false, 0 );
}

// ================================ //
//
void DefaultAssets::Release()
{
	delete LAYOUT_POSITION_NORMAL_COORD;
	delete LAYOUT_POSITION_COORD;
	delete LAYOUT_POSITION_COLOR;
	delete LAYOUT_COORD_COLOR;
}