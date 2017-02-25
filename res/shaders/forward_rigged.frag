#version 300 es
precision lowp float; 
struct DirectionalLight
{
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
    vec3 Direction;
}; 
uniform sampler2D tex;
uniform sampler2D ntex;
uniform sampler2D stex;
uniform vec3 gEyeWorldPos;

uniform vec3 gLightDirection;
uniform float gAmbientAdditive;

in vec2 fragTexCoord;
in vec3 vv3worldpos;
in vec3 vv3normal; 
in vec3 vv3tangent; 

out vec4 ragColor;

vec3 CalcBumpedNormal()
{
    vec3 Normal = normalize(vv3normal);
    vec3 Tangent = normalize(vv3tangent);
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    vec3 Bitangent = cross(Tangent, Normal);
    vec3 BumpMapNormal = texture2D(ntex, fragTexCoord).xyz;
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);
    vec3 NewNormal;
    mat3 TBN = mat3(Tangent, Bitangent, Normal);
    NewNormal = TBN * BumpMapNormal;
    NewNormal = normalize(NewNormal);
    return NewNormal;
}
void main() { 
    vec3 Normal = CalcBumpedNormal();
    //uniform DirectionalLight gDirectionalLight;
    vec4 ambientAdditive = vec4(0.6,0.6,0.7,1.0) * gAmbientAdditive;
    DirectionalLight gDirectionalLight;
    gDirectionalLight.Color = vec3(1.0,1.0,1.0);
    gDirectionalLight.AmbientIntensity = 0.2;
    gDirectionalLight.DiffuseIntensity = 0.2;
    gDirectionalLight.Direction = gLightDirection;
    vec4 AmbientColor = vec4(gDirectionalLight.Color * 
            gDirectionalLight.AmbientIntensity, 1.0) + ambientAdditive;
    float DiffuseFactor = dot(normalize(Normal), 
            -gDirectionalLight.Direction);
    vec4 SpecularColor = vec4(0, 0, 0, 0);

    vec4 DiffuseColor;
    if (DiffuseFactor > 0.0) {
        DiffuseColor = vec4(gDirectionalLight.Color * 
                gDirectionalLight.DiffuseIntensity * DiffuseFactor, 1.0);

        vec3 VertexToEye = normalize(gEyeWorldPos - vv3worldpos);
        vec3 LightReflect = normalize(reflect(gDirectionalLight.Direction, 
                    Normal));
        float SpecularFactor = dot(VertexToEye, LightReflect);
        if (SpecularFactor > 0.0) {
            SpecularFactor = pow(SpecularFactor, 1.0);
            SpecularColor = vec4(gDirectionalLight.Color * 4.0 * SpecularFactor, 
                    1.0) * texture2D(stex, fragTexCoord);
        }
    }
    else {
        DiffuseColor = vec4(0, 0, 0, 0);
    }
    vec4  c= texture2D(tex, fragTexCoord);
    vec4  z = vec4(0,0,0,0);
    //    gl_FragColor = vv4color*z + c; 
    ragColor = c* (AmbientColor + DiffuseColor + SpecularColor);
//    ragColor = vec4(Normal, 1.0);
}
