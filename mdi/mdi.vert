#version 330 core

//#extension GL_ARB_shader_draw_parameters : require

in  vec3 in_Position;
in  int in_Instance;
out vec3 ex_Color;

void main(void) {
   gl_Position = vec4(in_Position, 1.0);

//   int instance_id = gl_BaseInstanceARB;
   int instance_id = in_Instance;
//   int instance_id = gl_InstanceID;
//   int instance_id = gl_DrawIDARB;
//   int instance_id = 0;

   if (instance_id == 0)
       ex_Color = vec3(1.0, 0.0, 0.0);
   else if (instance_id == 1)
       ex_Color = vec3(0.0, 1.0, 0.0);
   else if (instance_id == 2)
       ex_Color = vec3(0.0, 0.0, 1.0);
   else if (instance_id == 3)
       ex_Color = vec3(1.0, 1.0, 0.0);
   else if (instance_id == 4)
       ex_Color = vec3(1.0, 0.0, 1.0);
   else if (instance_id == 5)
       ex_Color = vec3(0.0, 1.0, 1.0);
   else
       ex_Color = vec3(1.0, 1.0, 1.0);
}
