#pragma once
#ifndef _MISC_CONFIG_H_
#define _MISC_CONFIG_H_
#include <platform.h>

#if defined(_MSC_VER)
# define DECL_EXPORT __declspec(dllexport)
# define DECL_IMPORT __declspec(dllimport)
#endif

#if !defined(MISC_STATIC)
# if defined(BUILD_MISC_LIB)
# define MISC_EXPORTS DECL_EXPORT
# else
# define MISC_EXPORTS DECL_IMPORT
# endif
#else
# define MISC_EXPORTS
#endif

#endif