#version 330 core

in  vec3 in_Position;
in  vec3 in_Normal;
in  vec2 in_UV;
in  int in_Instance;
out vec3 ex_Color;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

void main(void) {
//   gl_Position = proj * view * world * vec4(in_Position, 1.0);
   gl_Position = vec4(in_Position, 1.0);
   int instance_id = in_Instance;
   ex_Color = vec3(1.0, 1.0, 1.0);
}
