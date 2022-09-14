#include "main.h"

static std::unordered_map<std::string, shaderc_shader_kind> kindMap = {
    {"vert", shaderc_shader_kind::shaderc_glsl_vertex_shader},
    {"frag", shaderc_shader_kind::shaderc_glsl_fragment_shader},
};

using SPIRV = std::vector<std::uint32_t>;
struct spirv_result {
    SPIRV spirv_bytes{};
    bool ok = false;
    std::string message{};
};

void inject_preamble(std::string& source) {
    const std::string preamble = R"(#ifdef SPIRV
#define texture2D texture
#define gl_FragColor FragColor
#endif
)";

    // look for the version directive first
    const auto n = source.find("#version");

    // if we found it, inject after
    if (n != std::string::npos) {
        if (const auto l = source.substr(n).find("\n"); l != std::string::npos) {
            source = source.substr(0, l+1) + preamble + source.substr(l + 1);

            std::cout << source << "\n";
            return;
        }
    }

    // otherwise, also inject a default version
    source = "#version 310 es\n" + preamble + source;

    //std::cout << source << "\n";
}

spirv_result compile_glsl(const std::string& source_name, shaderc_shader_kind kind,
    const std::string& source, bool optimize = true) noexcept
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetTargetSpirv(shaderc_spirv_version::shaderc_spirv_version_1_0);
    options.SetTargetEnvironment(shaderc_target_env::shaderc_target_env_opengl,
        shaderc_env_version::shaderc_env_version_opengl_4_5);

    options.AddMacroDefinition("SPIRV");
    options.SetGenerateDebugInfo();

    shaderc::PreprocessedSourceCompilationResult result =
        compiler.PreprocessGlsl(source, kind, source_name.c_str(), options);

    spirv_result res{};

    if (result.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success) {
        res.message = "PreprocessGlsl: " + result.GetErrorMessage();
        return res;
    }

    if (optimize) {
        // :)
        options.SetOptimizationLevel(shaderc_optimization_level::shaderc_optimization_level_size);
    }

    
    shaderc::SpvCompilationResult compileResult = compiler.CompileGlslToSpv(
        source, kind, source_name.c_str(), options);
    
    if (compileResult.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success) {
        res.message = "CompileGlslToSpv: " + compileResult.GetErrorMessage();
        return res;
    }

    res.ok = true;
    res.spirv_bytes = SPIRV{ compileResult.cbegin(), compileResult.cend()};
    return res;
}

std::string msl_transpile(SPIRV&& spirvBytes) {
    spirv_cross::CompilerMSL msl(std::move(spirvBytes));
    spirv_cross::CompilerMSL::Options options;
    options.platform = decltype(options)::Platform::iOS;
    msl.set_msl_options(options);

    return msl.compile();
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: filename.glsl [vert|frag] outfile.msl";
        return -1;
    }

    const auto filename = std::string(argv[1]);
    const auto kind = kindMap.find({argv[2]});
    if (kind == kindMap.end()) {
        std::cerr << "Unknown shader stage " + std::string(argv[1]) + "\n";
        return -1;
    }

    const auto shader_kind = kind->second;

    auto file = std::ifstream(filename, std::fstream::in);
    if (!file.is_open()) {
        std::cerr << "Failed to open file " + filename + "\n";
        return -1;
    }

    file.seekg(0);
    std::stringstream data;
    data << file.rdbuf();
    std::string shader_source = data.str();
    inject_preamble(shader_source);

    auto result = compile_glsl(filename, shader_kind, shader_source);
    if (!result.ok) {
        std::cerr << result.message;
        return -1;
    }

    const auto msl = msl_transpile(std::move(result.spirv_bytes));
    std::ofstream ofs(std::string(argv[3]), std::ios::out);
    ofs << msl;

    return 0;
}