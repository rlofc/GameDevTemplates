#version 300 es
in vec3 av4position; 
//in vec4 av4color; 
in vec2 av2texcoord; 
in vec3 av3normal;
in vec3 av3tangent;
in vec3 av3bindices;
in vec3 av3bweights;
in mat4 av4transform;
//in mat4 av4world;

uniform vec4 quat_reals[64];
uniform vec4 quat_duals[64];

//out int InstanceID; 

uniform mat4 mvp; 
uniform mat4 otr; 

out vec4 vv4color; 
out vec2 fragTexCoord; 
out vec3 vv3normal;
out vec3 vv3tangent;
out vec3 vv3worldpos;

vec3 quat_dual_mul_pos(vec4 real, vec4 dual, vec3 v) {
  return v + 2.0 * cross(real.xyz, cross(real.xyz, v) + real.w*v) +
             2.0 * (real.w * dual.xyz - dual.w * real.xyz + cross(real.xyz, dual.xyz));
}

vec3 quat_dual_mul_rot(vec4 real, vec4 dual, vec3 v) {
      return v + 2.0 * cross(real.xyz, cross(real.xyz, v) + real.w*v);
}

void main(void) { 
  vec4 real = quat_reals[int(av3bindices.x)] * av3bweights.x +
              quat_reals[int(av3bindices.y)] * av3bweights.y +
              quat_reals[int(av3bindices.z)] * av3bweights.z;
  
  vec4 dual = quat_duals[int(av3bindices.x)] * av3bweights.x +
              quat_duals[int(av3bindices.y)] * av3bweights.y +
              quat_duals[int(av3bindices.z)] * av3bweights.z;
  
  dual = dual / length(real);
  real = real / length(real);
  
  vec3 blendpos    = quat_dual_mul_pos(real, dual, av4position);
  vec3 blendnorm   = quat_dual_mul_rot(real, dual, av3normal);
  vec3 blendtang   = quat_dual_mul_rot(real, dual, av3tangent);
  mat4 otrx = av4transform;
  
  blendnorm   = mat3(otrx) * blendnorm;
  blendtang   = mat3(otrx) * blendtang;


  vec4 world_pos = otrx * vec4(blendpos, 1.0);
  
  gl_Position = (mvp * world_pos);
fragTexCoord = av2texcoord;
vv3normal = blendnorm.xyz;//(otrx * vec4(av3normal,0)).xyz;
vv3tangent = blendtang.xyz;//(otrx *vec4(av3tangent,0)).xyz;
vv3worldpos = world_pos.xyz;//(otrx * vec4(av4position,1)).xyz;
}
