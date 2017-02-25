#version 300 es
precision lowp float; 

out vec4 ov4_frag_color;
in vec2 vv2_texcoord;

uniform sampler2D s2d_position;
uniform sampler2D s2d_normal;
uniform sampler2D s2d_albedospec;
uniform sampler2D s2d_fog;

struct t_light {
    vec3 position;
    vec3 color;
    float linear;
    float quadratic;
};

const int NR_LIGHTS = 32;
uniform vec3 uv3_viewpos;
uniform t_light lights[NR_LIGHTS];

void main()
{             
    // Retrieve data from gbuffer
    vec3 g_frag_pos = texture(s2d_position, vv2_texcoord).rgb;
    vec3 g_normal = texture(s2d_normal, vv2_texcoord).rgb;
    vec3 g_diffuse = texture(s2d_albedospec, vv2_texcoord).rgb;
    float g_specular = texture(s2d_albedospec, vv2_texcoord).a;

    // Then calculate lighting as usual
    vec3 lighting  = g_diffuse * 0.1; // hard-coded ambient component
    vec3 view_dir  = normalize(uv3_viewpos - g_frag_pos);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // Diffuse
        vec3 light_dir = normalize(lights[i].position - g_frag_pos);
        vec3 diffuse = max(dot(g_normal, light_dir), 0.0) * g_diffuse * lights[i].color;
        // Specular
        vec3 halfway_dir = normalize(light_dir + view_dir);  
        float spec = pow(max(dot(g_normal, halfway_dir), 0.0), 16.0);
        vec3 specular = lights[i].color * spec * g_specular;
        // Attenuation
        float distance = length(lights[i].position - g_frag_pos);
        float attenuation = 1.0 / (1.0 + (lights[i].linear) * distance + (lights[i].quadratic) * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }    
    const vec4 fog_color = vec4(0.8,0.85,0.77,1.0); // white
    float fog_amount = texture(s2d_fog, vv2_texcoord).a;
    ov4_frag_color = vec4(lighting, 1.0) * fog_amount + fog_color * (1.0 - fog_amount);
}
