#pragma once

#include "Common/RTTR.h"
#include "Common/TypesDefinitions.h"
#include <DirectXMath.h>

class EngineObject;

namespace EditorPlugin
{

	public enum PropertyType
	{
		PropertyFloat,
		PropertyFloat2,
		PropertyFloat3,
		PropertyFloat4,
		PropertyUInt,
		PropertyInt,
		PropertyBool,
		PropertyDouble,
		PropertyActor,
		PropertyCategory,
		PropertyResource,
		PropertyString,
		PropertyWString,
		PropertyEnum,
		PropertyArray,
		PropertyUnknown
	};


	template< typename PropertyType >
	PropertyType		GetPropertyValue( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject );

	template< typename PropertyType >
	void				SetPropertyValue( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject, PropertyType newValue );

	/**@brief Helper class to extract private property_wrapper_base from rttr::property.*/
	public class RTTRPropertyRapist
	{
		friend ref class PropertyWrapper;
		friend ref class FloatPropertyWrapper;
		friend ref class Float2PropertyWrapper;
		friend ref class Float3PropertyWrapper;
		friend ref class Float4PropertyWrapper;
		friend ref class IntPropertyWrapper;
		friend ref class UIntPropertyWrapper;
		friend ref class BoolPropertyWrapper;
		friend ref class ActorClassMetaInfo;
		friend ref class CategoryPropertyWrapper;
		friend ref class ObjectPropertyWrapper;
		friend ref class XMFloatPropertyWrapper;
		friend ref class CategoryLessPropertyWrapper;
		friend ref class StringPropertyWrapper;
		friend ref class WStringPropertyWrapper;
		friend ref class TexturePropertyWrapper;
		friend ref class EnumPropertyWrapper;
		friend ref class ArrayPropertyWrapper;
		friend ref class ArrayElementPropertyWrapper;

		friend DirectX::XMFLOAT2	GetPropertyValue< DirectX::XMFLOAT2 >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject );
		friend void					SetPropertyValue< DirectX::XMFLOAT2 >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject, DirectX::XMFLOAT2 newValue );

		friend DirectX::XMFLOAT3	GetPropertyValue< DirectX::XMFLOAT3 >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject );
		friend void					SetPropertyValue< DirectX::XMFLOAT3 >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject, DirectX::XMFLOAT3 newValue );

		friend DirectX::XMFLOAT4	GetPropertyValue< DirectX::XMFLOAT4 >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject );
		friend void					SetPropertyValue< DirectX::XMFLOAT4 >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject, DirectX::XMFLOAT4 newValue );

		friend int					GetPropertyValue< int >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject );
		friend void					SetPropertyValue< int >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject, int newValue );

		friend uint32				GetPropertyValue< uint32 >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject );
		friend void					SetPropertyValue< uint32 >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject, uint32 newValue );

		friend float				GetPropertyValue< float >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject );
		friend void					SetPropertyValue< float >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject, float newValue );

		friend double				GetPropertyValue< double >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject );
		friend void					SetPropertyValue< double >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject, double newValue );

		friend bool					GetPropertyValue< bool >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject );
		friend void					SetPropertyValue< bool >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject, bool newValue );

		friend std::string 			GetPropertyValue< std::string >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject );
		friend void					SetPropertyValue< std::string >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject, std::string newValue );

		friend std::wstring 		GetPropertyValue< std::wstring >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject );
		friend void					SetPropertyValue< std::wstring >( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject, std::wstring newValue );

		inline static rttr::property									MakeProperty		( const rttr::detail::property_wrapper_base* wrapper )
		{		return rttr::property( wrapper );		}

		inline static const rttr::detail::property_wrapper_base*		GetWrapperBase	( rttr::property prop )
		{		return prop.m_wrapper;		}
	};

	/**@brief */
	template< typename PropertyType >
	PropertyType	GetPropertyValue( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject )
	{
		rttr::property prop = RTTRPropertyRapist::MakeProperty( metaProperty );

		// Create variant with void* type and convert it to proper type.
		rttr::variant declaringObject( refObject.ToPointer() );
		bool success = declaringObject.unsafe_convert_void( prop.get_declaring_type_ptr() );
		
		assert( success );

		auto value = prop.get_value( declaringObject );
		return value.get_value< PropertyType >();
	}

	/**@brief */
	template< typename PropertyType >
	void		SetPropertyValue( const rttr::detail::property_wrapper_base* metaProperty, System::IntPtr refObject, PropertyType newValue )
	{
		rttr::property prop = RTTRPropertyRapist::MakeProperty( metaProperty );

		// Create variant with void* type and convert it to proper type.
		rttr::variant declaringObject( refObject.ToPointer() );
		//prop.get_declaring_type().
		bool success = declaringObject.unsafe_convert_void( prop.get_declaring_type_ptr() );

		assert( success );

		prop.set_value( declaringObject, newValue );
	}

}


