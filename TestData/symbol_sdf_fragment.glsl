// https://github.com/maplibre/maplibre-gl-js

#define SDF_PX 8.0

#ifdef SPIRV
precision highp float;
precision highp int;

layout(binding = 0) uniform _Uniforms {
    bool u_is_halo;
    highp float u_gamma_scale;
    lowp float u_device_pixel_ratio;
    bool u_is_text;

    highp vec4 fill_color;
    highp vec4 halo_color;
    lowp float opacity;
    lowp float halo_width;
    lowp float halo_blur;
};

layout(binding = 1) uniform sampler2D u_texture;
#else
uniform bool u_is_halo;
uniform sampler2D u_texture;
uniform highp float u_gamma_scale;
uniform lowp float u_device_pixel_ratio;
uniform bool u_is_text;
#endif

#ifdef SPIRV
layout(location = 0) in vec2 v_data0;
layout(location = 1) in vec3 v_data1;
layout(location = 0) out vec4 FragColor;
#else
varying vec2 v_data0;
varying vec3 v_data1;

#pragma mapbox: define highp vec4 fill_color
#pragma mapbox: define highp vec4 halo_color
#pragma mapbox: define lowp float opacity
#pragma mapbox: define lowp float halo_width
#pragma mapbox: define lowp float halo_blur
#endif

void main() {
#ifndef SPIRV
    #pragma mapbox: initialize highp vec4 fill_color
    #pragma mapbox: initialize highp vec4 halo_color
    #pragma mapbox: initialize lowp float opacity
    #pragma mapbox: initialize lowp float halo_width
    #pragma mapbox: initialize lowp float halo_blur
#endif

    float EDGE_GAMMA = 0.105 / u_device_pixel_ratio;

    vec2 tex = v_data0.xy;
    float gamma_scale = v_data1.x;
    float size = v_data1.y;
    float fade_opacity = v_data1[2];

    float fontScale = u_is_text ? size / 24.0 : size;

    lowp vec4 color = fill_color;
    highp float gamma = EDGE_GAMMA / (fontScale * u_gamma_scale);
    lowp float buff = (256.0 - 64.0) / 256.0;
    if (u_is_halo) {
        color = halo_color;
        gamma = (halo_blur * 1.19 / SDF_PX + EDGE_GAMMA) / (fontScale * u_gamma_scale);
        buff = (6.0 - halo_width / fontScale) / SDF_PX;
    }

    lowp float dist = texture2D(u_texture, tex).a;
    highp float gamma_scaled = gamma * gamma_scale;
    highp float alpha = smoothstep(buff - gamma_scaled, buff + gamma_scaled, dist);

    gl_FragColor = color * (alpha * opacity * fade_opacity);

#ifdef OVERDRAW_INSPECTOR
    gl_FragColor = vec4(1.0);
#endif
}