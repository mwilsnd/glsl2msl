#pragma once
#include<stdint.h>
#include<vector>
#include<stdio.h>
#include<iostream>
#include<unordered_map>
#include<string_view>
#include<sstream>
#include<fstream>

#pragma warning(push)
#pragma warning(disable : 26439)
#include<shaderc/shaderc.hpp>
#include<spirv_cross/external_interface.h>
#include<spirv_msl.hpp>
#include<spirv_glsl.hpp>
#pragma warning(pop)