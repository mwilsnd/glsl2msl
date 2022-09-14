# GLSL2MSL
To get started, clone the repo (`-recursive`) and build using CMake. Run example: ```./glsl2msl TestData/symbol_sdf_fragment.glsl frag test_symbol_sdf_fragment.msl```. View test input and converted output in `TestData/`.

The converter will define `SPIRV` for you to perform conditionals with inside your GLSL. A preamble will also be injected along with a default version directive if none is found. By default, this is `#version 310 es`. The preamble:
* Aliases texture2D to texture
* Aliases gl_FragColor to FragColor