#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <FreeImage/FreeImage.h>

#include "Shaders/LoadShaders.h"
#include "My_Shading.h"
GLuint h_ShaderProgram_simple, h_ShaderProgram_TXPS, h_ShaderProgram_GS; // handles to shader programs

// for simple shaders
GLint loc_ModelViewProjectionMatrix_simple, loc_primitive_color;

// for Phong Shading (Textured) shaders
#define NUMBER_OF_LIGHT_SUPPORTED 4
GLint loc_global_ambient_color, loc_global_ambient_color_GS;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_light_Parameters loc_light_GS[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;
loc_Material_Parameters loc_material_GS;

GLint loc_ModelViewProjectionMatrix_TXPS, loc_ModelViewMatrix_TXPS, loc_ModelViewMatrixInvTrans_TXPS;
GLint loc_ModelViewProjectionMatrix_GS, loc_ModelViewMatrix_GS, loc_ModelViewMatrixInvTrans_GS;
GLint loc_texture, loc_flag_texture_mapping, loc_flag_fog;

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp> // inverseTranspose, etc.
glm::mat4 ModelViewProjectionMatrix, ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;
glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

#include "Geometry.h"

GLint loc_screen_effect, loc_screen_frequency, loc_screen_width;
GLint loc_blind_effect, loc_blind_num;
GLfloat loc_u_fragment_alpha, loc_u_flag_blending;
int window_width = 1200;
int window_height = 800;
unsigned int flag_fog = 0;

/*********************************  START: camera *********************************/
typedef struct _Camera {
	float pos[3];
	float uaxis[3], vaxis[3], naxis[3];
	float fovy, aspect_ratio, near_c, far_c;
	int move;
} Camera;

Camera camera;
int cur_camera = 0;
glm::mat4 position_dragon;

void set_up_scene_lights2() {
	light[2].position[0] = camera.pos[0]; light[2].position[1] = camera.pos[1]; // spot light position in WC
	light[2].position[2] = camera.pos[2]; light[2].position[3] = 1.0f;

	light[2].spot_direction[0] = -camera.naxis[0]; light[2].spot_direction[1] = -camera.naxis[1]; // spot light direction in WC
	light[2].spot_direction[2] = -camera.naxis[2];

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_light[0].light_on, light[0].light_on);
	glUniform4fv(loc_light[0].position, 1, light[0].position);
	glUniform4fv(loc_light[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light[0].specular_color, 1, light[0].specular_color);

	glUniform1i(loc_light[1].light_on, light[1].light_on);
	glm::vec4 position_EC = ViewMatrix * glm::vec4(light[1].position[0], light[1].position[1], light[1].position[2], light[1].position[3]);
	glUniform4fv(loc_light[1].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[1].ambient_color, 1, light[1].ambient_color);
	glUniform4fv(loc_light[1].diffuse_color, 1, light[1].diffuse_color);
	glUniform4fv(loc_light[1].specular_color, 1, light[1].specular_color);
	glm::vec3 direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[1].spot_direction[0], light[1].spot_direction[1],
		light[1].spot_direction[2]);
	glUniform3fv(loc_light[1].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light[1].spot_cutoff_angle, light[1].spot_cutoff_angle);
	glUniform1f(loc_light[1].spot_exponent, light[1].spot_exponent);

	glUniform1i(loc_light[2].light_on, light[2].light_on);
	position_EC = ViewMatrix * glm::vec4(light[2].position[0], light[2].position[1], light[2].position[2], light[2].position[3]);
	glUniform4fv(loc_light[2].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[2].ambient_color, 1, light[2].ambient_color);
	glUniform4fv(loc_light[2].diffuse_color, 1, light[2].diffuse_color);
	glUniform4fv(loc_light[2].specular_color, 1, light[2].specular_color);
	direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[2].spot_direction[0], light[2].spot_direction[1], light[2].spot_direction[2]);
	glUniform3fv(loc_light[2].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light[2].spot_cutoff_angle, light[2].spot_cutoff_angle);
	glUniform1f(loc_light[2].spot_exponent, light[2].spot_exponent);

	glUniform1i(loc_light[3].light_on, light[3].light_on);
	position_EC = ViewMatrix * glm::vec4(light[3].position[0], light[3].position[1], light[3].position[2], light[3].position[3]);
	glUniform4fv(loc_light[3].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[3].ambient_color, 1, light[3].ambient_color);
	glUniform4fv(loc_light[3].diffuse_color, 1, light[3].diffuse_color);
	glUniform4fv(loc_light[3].specular_color, 1, light[3].specular_color);
	direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[2].spot_direction[0], light[3].spot_direction[1], light[3].spot_direction[2]);
	glUniform3fv(loc_light[3].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light[3].spot_cutoff_angle, light[3].spot_cutoff_angle);
	glUniform1f(loc_light[3].spot_exponent, light[3].spot_exponent);
	glUseProgram(0);


	glUseProgram(h_ShaderProgram_GS);
	glUniform1i(loc_light_GS[0].light_on, light[0].light_on);
	glUniform4fv(loc_light_GS[0].position, 1, light[0].position);
	glUniform4fv(loc_light_GS[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light_GS[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light_GS[0].specular_color, 1, light[0].specular_color);

	glUniform1i(loc_light[1].light_on, light[1].light_on);
	// need to supply position in EC for shading
	position_EC = ViewMatrix * glm::vec4(light[1].position[0], light[1].position[1],
		light[1].position[2], light[1].position[3]);
	glUniform4fv(loc_light_GS[1].position, 1, &position_EC[0]);
	glUniform4fv(loc_light_GS[1].ambient_color, 1, light[1].ambient_color);
	glUniform4fv(loc_light_GS[1].diffuse_color, 1, light[1].diffuse_color);
	glUniform4fv(loc_light_GS[1].specular_color, 1, light[1].specular_color);
	// need to supply direction in EC for shading in this example shader
	direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[1].spot_direction[0], light[1].spot_direction[1], light[1].spot_direction[2]);
	glUniform3fv(loc_light_GS[1].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light_GS[1].spot_cutoff_angle, light[1].spot_cutoff_angle);
	glUniform1f(loc_light_GS[1].spot_exponent, light[1].spot_exponent);

	glUniform1i(loc_light_GS[2].light_on, light[2].light_on);
	position_EC = ViewMatrix * glm::vec4(light[2].position[0], light[2].position[1], light[2].position[2], light[2].position[3]);
	glUniform4fv(loc_light_GS[2].position, 1, &position_EC[0]);
	glUniform4fv(loc_light_GS[2].ambient_color, 1, light[2].ambient_color);
	glUniform4fv(loc_light_GS[2].diffuse_color, 1, light[2].diffuse_color);
	glUniform4fv(loc_light_GS[2].specular_color, 1, light[2].specular_color);
	direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[2].spot_direction[0], light[2].spot_direction[1], light[2].spot_direction[2]);
	glUniform3fv(loc_light_GS[2].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light_GS[2].spot_cutoff_angle, light[2].spot_cutoff_angle);
	glUniform1f(loc_light_GS[2].spot_exponent, light[2].spot_exponent);

	glUniform1i(loc_light_GS[3].light_on, light[3].light_on);
	position_EC = ViewMatrix * glm::vec4(light[3].position[0], light[3].position[1], light[3].position[2], light[3].position[3]);
	glUniform4fv(loc_light_GS[3].position, 1, &position_EC[0]);
	glUniform4fv(loc_light_GS[3].ambient_color, 1, light[3].ambient_color);
	glUniform4fv(loc_light_GS[3].diffuse_color, 1, light[3].diffuse_color);
	glUniform4fv(loc_light_GS[3].specular_color, 1, light[3].specular_color);
	direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[2].spot_direction[0], light[3].spot_direction[1], light[3].spot_direction[2]);
	glUniform3fv(loc_light_GS[3].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light_GS[3].spot_cutoff_angle, light[3].spot_cutoff_angle);
	glUniform1f(loc_light_GS[3].spot_exponent, light[3].spot_exponent);
	glUseProgram(0);
}

void set_ViewMatrix_from_camera_frame(void) {
	glm::vec4 position_EC;
	glm::vec3 direction_EC;

	ViewMatrix = glm::mat4(camera.uaxis[0], camera.vaxis[0], camera.naxis[0], 0.0f,
		camera.uaxis[1], camera.vaxis[1], camera.naxis[1], 0.0f,
		camera.uaxis[2], camera.vaxis[2], camera.naxis[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	ViewMatrix = glm::translate(ViewMatrix, glm::vec3(-camera.pos[0], -camera.pos[1], -camera.pos[2]));
	set_up_scene_lights2();
}

void initialize_camera(void) {
	camera.pos[0] = 1500.0f; camera.pos[1] = 150.0f;  camera.pos[2] = 0.0f;
	camera.uaxis[0] = camera.uaxis[1] = 0.0f; camera.uaxis[2] = -1.0f;
	camera.vaxis[0] = camera.vaxis[2] = 0.0f; camera.vaxis[1] = 1.0f;
	camera.naxis[1] = camera.naxis[2] = 0.0f; camera.naxis[0] = 1.0f;
	camera.move = 1;
	camera.fovy = 30.0f, camera.aspect_ratio = window_width / (float)window_height; camera.near_c = 0.1f; camera.far_c = 3000.0f;

	set_ViewMatrix_from_camera_frame();
	ProjectionMatrix = glm::perspective(TO_RADIAN * camera.fovy, camera.aspect_ratio, camera.near_c, camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void initialize_camera2(void) {
	// camera 2
	camera.pos[0] = 1000.0f; camera.pos[1] = 150.0f;  camera.pos[2] = -1000.0f;
	camera.naxis[0] = sqrt(1.0f / 2.0f); camera.naxis[1] = 0; camera.naxis[2] = -1 * sqrt(1.0f / 2.0f);
	camera.uaxis[0] = -1 * sqrt(1.0f / 2.0f); camera.uaxis[1] = 0.0f; camera.uaxis[2] = -1 * sqrt(1.0f / 2.0f);
	camera.vaxis[0] = 0.0f;; camera.vaxis[1] = 1.0f; camera.vaxis[2] = 0.0f;;
	camera.move = 1;
	camera.fovy = 30.0f, camera.aspect_ratio = window_width / (float)window_height; camera.near_c = 0.1f; camera.far_c = 30000.0f;

	set_ViewMatrix_from_camera_frame();
	ProjectionMatrix = glm::perspective(TO_RADIAN * camera.fovy, camera.aspect_ratio, camera.near_c, camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void initialize_camera3(void) {
	// camera 3
	camera.pos[0] = 0.0f; camera.pos[1] = 2500.0f;  camera.pos[2] = 0.0f;
	camera.uaxis[0] = 0.0f; camera.uaxis[1] = 0.0f; camera.uaxis[2] = -1.0f;
	camera.vaxis[0] = -1.0f; camera.vaxis[2] = 0.0f; camera.vaxis[1] = 0.0f;
	camera.naxis[0] = 0.0f; camera.naxis[1] = 1.0f; camera.naxis[2] = 0.0f;
	camera.move = 1;
	camera.fovy = 30.0f, camera.aspect_ratio = window_width / (float)window_height; camera.near_c = 0.1f; camera.far_c = 30000.0f;

	set_ViewMatrix_from_camera_frame();
	ProjectionMatrix = glm::perspective(TO_RADIAN * camera.fovy, camera.aspect_ratio, camera.near_c, camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

enum taxes { X_AXIS, Y_AXIS, Z_AXIS };
#define CAM_TSPEED 0.05f
void renew_cam_position(int del, int d) {
	if (camera.move != 1) return;
	switch (d) {
	case X_AXIS:
		camera.pos[0] += CAM_TSPEED * del * (camera.uaxis[0]);
		camera.pos[1] += CAM_TSPEED * del * (camera.uaxis[1]);
		camera.pos[2] += CAM_TSPEED * del * (camera.uaxis[2]);
		break;
	case Y_AXIS:
		camera.pos[0] += CAM_TSPEED * del * (camera.vaxis[0]);
		camera.pos[1] += CAM_TSPEED * del * (camera.vaxis[1]);
		camera.pos[2] += CAM_TSPEED * del * (camera.vaxis[2]);
		break;
	case Z_AXIS:
		camera.pos[0] += CAM_TSPEED * del * (-camera.naxis[0]);
		camera.pos[1] += CAM_TSPEED * del * (-camera.naxis[1]);
		camera.pos[2] += CAM_TSPEED * del * (-camera.naxis[2]);
		break;
	}
	set_ViewMatrix_from_camera_frame();
	ProjectionMatrix = glm::perspective(TO_RADIAN * camera.fovy, camera.aspect_ratio, camera.near_c, camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	glutPostRedisplay();
}

#define CAM_RSPEED 0.1f
enum raxes { V_AXIS, U_AXIS, N_AXIS };
int flag_rotation_axis = 0;
void renew_cam_orientation_rotation_around_axis(int angle) {
	glm::mat3 RotationMatrix;
	glm::vec3 direction;

	switch (flag_rotation_axis) {
	case V_AXIS:
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle, glm::vec3(camera.vaxis[0], camera.vaxis[1], camera.vaxis[2])));
		direction = RotationMatrix * glm::vec3(camera.uaxis[0], camera.uaxis[1], camera.uaxis[2]);
		camera.uaxis[0] = direction.x; camera.uaxis[1] = direction.y; camera.uaxis[2] = direction.z;
		direction = RotationMatrix * glm::vec3(camera.naxis[0], camera.naxis[1], camera.naxis[2]);
		camera.naxis[0] = direction.x; camera.naxis[1] = direction.y; camera.naxis[2] = direction.z;
		break;
	case U_AXIS:
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle, glm::vec3(camera.uaxis[0], camera.uaxis[1], camera.uaxis[2])));
		direction = RotationMatrix * glm::vec3(camera.vaxis[0], camera.vaxis[1], camera.vaxis[2]);
		camera.vaxis[0] = direction.x; camera.vaxis[1] = direction.y; camera.vaxis[2] = direction.z;
		direction = RotationMatrix * glm::vec3(camera.naxis[0], camera.naxis[1], camera.naxis[2]);
		camera.naxis[0] = direction.x; camera.naxis[1] = direction.y; camera.naxis[2] = direction.z;
		break;
	case N_AXIS:
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle, glm::vec3(camera.naxis[0], camera.naxis[1], camera.naxis[2])));
		direction = RotationMatrix * glm::vec3(camera.uaxis[0], camera.uaxis[1], camera.uaxis[2]);
		camera.uaxis[0] = direction.x; camera.uaxis[1] = direction.y; camera.uaxis[2] = direction.z;
		direction = RotationMatrix * glm::vec3(camera.vaxis[0], camera.vaxis[1], camera.vaxis[2]);
		camera.vaxis[0] = direction.x; camera.vaxis[1] = direction.y; camera.vaxis[2] = direction.z;
		break;
	}
}
/*********************************  END: camera *********************************/


// callbacks
float PRP_distance_scale[6] = { 0.5f, 1.0f, 2.5f, 5.0f, 10.0f, 20.0f };

unsigned int timestamp_scene = 0; // the global clock in the scene
unsigned int color_effect = 0;

int flag_blend_mode = 0; // 0: blending off, 1: blending on
int flag_back_to_front = 1;

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	draw_axes();
	glLineWidth(1.0f);

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_texture, TEXTURE_ID_FLOOR);
  	set_material_floor();
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-500.0f, 0.0f, 500.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_floor();
	
	set_material_spider();
	glUniform1i(loc_texture, TEXTURE_ID_SPIDER);
 	ModelViewMatrix = glm::rotate(ViewMatrix, -rotation_angle_tiger, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(200.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_spider();

	set_material_dragon();
	glUniform1i(loc_texture, 3);
	glUniform1i(loc_flag_fog, 0);
	float temp_dragon_loc;
	if (dragon_moving)
		temp_dragon_loc = dragon_loc;
	else
		temp_dragon_loc = before_dragon_loc;
	ModelViewMatrix = glm::rotate(ViewMatrix, temp_dragon_loc * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(0.0f, 400.0f+ sin(temp_dragon_loc / 20)*200, -500.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(-10.0f, 10.0f, 10.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	position_dragon = glm::mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
	position_dragon = glm::rotate(position_dragon, temp_dragon_loc * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	position_dragon = glm::translate(position_dragon, glm::vec3(0.0f, 400.0f + sin(temp_dragon_loc / 20) *200, -500.0f));
	glm::vec4 posi = position_dragon * glm::vec4(0, 0, 0, 1);
	light[3].position[0] = posi[0]; light[3].position[1] = posi[1]; // spot light position in WC
	light[3].position[2] = posi[2]; light[3].position[3] = 1.0f;

	Light_Parameters tmp_light;
	float tmpr, tmpg, tmpb;
	if (color_effect) {
		tmpr = (int)tank_loc % 180;
		tmpg = ((int)tank_loc + 120) % 180;
		tmpb = ((int)tank_loc + 240) % 180;
		if (tmpr > 90)
			tmpr -= 90;
		if (tmpg > 90)
			tmpg -= 90;		
		if (tmpb > 90)
			tmpb -= 90;
		tmpr /= 90.0; tmpg /= 90.0; tmpb /= 90.0;
		tmp_light.ambient_color[0] = tmpr /3; tmp_light.ambient_color[1] = tmpg / 3;
		tmp_light.ambient_color[2] = tmpb / 3; tmp_light.ambient_color[3] = 1.0f;
		tmp_light.diffuse_color[0] = tmpr / 2; tmp_light.diffuse_color[1] = tmpg / 2;
		tmp_light.diffuse_color[2] = tmpb / 2; tmp_light.diffuse_color[3] = 1.0f;
		tmp_light.specular_color[0] = tmpr/1.2; tmp_light.specular_color[1] = tmpg/1.2;
		tmp_light.specular_color[2] = tmpb/1.2; tmp_light.specular_color[3] = 1.0f;
		glUniform4fv(loc_light[3].ambient_color, 1, tmp_light.ambient_color);
		glUniform4fv(loc_light[3].diffuse_color, 1, tmp_light.diffuse_color);
		glUniform4fv(loc_light[3].specular_color, 1, tmp_light.specular_color);
	}
	else {
		glUniform4fv(loc_light[3].ambient_color, 1, light[3].ambient_color);
		glUniform4fv(loc_light[3].diffuse_color, 1, light[3].diffuse_color);
		glUniform4fv(loc_light[3].specular_color, 1, light[3].specular_color);
	}
	glUniform1i(loc_light[3].light_on, light[3].light_on);
	glm::vec4 position_EC = ViewMatrix * glm::vec4(light[3].position[0], light[3].position[1], light[3].position[2], light[3].position[3]);
	glUniform4fv(loc_light[3].position, 1, &position_EC[0]);
	glm::vec3 direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[3].spot_direction[0], light[3].spot_direction[1], light[3].spot_direction[2]);
	glUniform3fv(loc_light[3].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light[3].spot_cutoff_angle, light[3].spot_cutoff_angle);
	glUniform1f(loc_light[3].spot_exponent, light[3].spot_exponent);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_dragon();

	set_material_ironman();
	if (iron_man_moving) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(300.0f, 100.0f, 400 * cos((iron_man_loc * TO_RADIAN))));
		if (iron_man_loc < 180)
			ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
		else
			ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	}
	else {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(300.0f, 100.0f, 400 * cos((before_iron_man_loc * TO_RADIAN))));
		if (before_iron_man_loc < 180)
			ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
		else
			ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	}
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 30.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_ironman();
	glUniform1i(loc_flag_fog, flag_fog);

	set_material_tank();
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, 0.0f, -200.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_tank();

	glUseProgram(h_ShaderProgram_GS);
	set_material_tank_GS();
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, 0.0f, 200.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_GS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_tank_GS();

	glUseProgram(0);
	glutSwapBuffers();
}

void timer_scene(int value) {
	timestamp_scene = (timestamp_scene + 1) % UINT_MAX;
	cur_frame_tiger = timestamp_scene % N_TIGER_FRAMES;
	rotation_angle_tiger = (timestamp_scene % 360)*TO_RADIAN;
	dragon_loc = (timestamp_scene - (int)before_dragon_loc) % (360*20);
	iron_man_loc = (timestamp_scene - (int)before_iron_man_loc) % 360;
	tank_loc = ((timestamp_scene - (int)before_tank_loc) % 720);
	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_blind_num, timestamp_scene);
	glUseProgram(0);

	glutPostRedisplay();
	if (flag_tiger_animation)
		glutTimerFunc(100, timer_scene, 0);
}

void keyboard(unsigned char key, int x, int y) {
	static int camera_mode = 1;
	static int flag_blind_effect = 0;

	glm::vec4 position_EC;
	glm::vec3 direction_EC;

	if ((key >= '0') && (key <= '0' + NUMBER_OF_LIGHT_SUPPORTED - 1)) {
		int light_ID = (int) (key - '0');

		glUseProgram(h_ShaderProgram_TXPS);
		light[light_ID].light_on = 1 - light[light_ID].light_on;
		glUniform1i(loc_light[light_ID].light_on, light[light_ID].light_on);
		glUseProgram(0);
		glUseProgram(h_ShaderProgram_GS);
		glUniform1i(loc_light_GS[light_ID].light_on, light[light_ID].light_on);
		glUseProgram(0);

		glutPostRedisplay();
		return;
	}
	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case 'v':
		flag_rotation_axis = V_AXIS;
		break;
	case 'u':
		flag_rotation_axis = U_AXIS;
		break;
	case 'n':
		flag_rotation_axis = N_AXIS;
		break;
	case 'd':
		renew_cam_position(100, X_AXIS);
		break;
	case 'a':
		renew_cam_position(-200, X_AXIS);
		break;
	case 'w':
		renew_cam_position(200, Z_AXIS);
		break;
	case 's':
		renew_cam_position(-200, Z_AXIS);
		break;
	case 'q':
		renew_cam_position(200, Y_AXIS);
		break;
	case 'z':
		renew_cam_position(-200, Y_AXIS);
		break;
	case 'c':
		if (!camera_mode) {
			initialize_camera();
			camera_mode = 1;
		}
		else if (camera_mode == 1){
			initialize_camera2();
			camera_mode = 2;
		}
		else {
			initialize_camera3();
			camera_mode = 0;
		}
		break;
/*
	case 'm':
		if (iron_man_moving) {
			iron_man_moving = 0;
			before_iron_man_loc = iron_man_loc;
		}
		else {
			iron_man_moving = 1;
			before_iron_man_loc = iron_man_loc;
		}
		break;
	case 'r':
		if (dragon_moving) {
			dragon_moving = 0;
			before_dragon_loc = dragon_loc;
		}
		else {
			dragon_moving = 1;
			before_dragon_loc = dragon_loc;
		}
		break;
	case 't':
		if (tank_moving) {
			tank_moving = 0;
			before_tank_loc = tank_loc;
		}
		else {
			tank_moving = 1;
			before_tank_loc = tank_loc;
		}
		break;*/
	case 'i':
		if (camera.move != 1)
			break;
		if (camera.fovy < 3)
			break;
		camera.fovy -= 3;
		ProjectionMatrix = glm::perspective(TO_RADIAN * camera.fovy, camera.aspect_ratio, camera.near_c, camera.far_c);
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
		break;
	case 'o':
		if (camera.move != 1)
			break;
		if (camera.fovy > 57)
			break;
		camera.fovy += 3;
		ProjectionMatrix = glm::perspective(TO_RADIAN * camera.fovy, camera.aspect_ratio, camera.near_c, camera.far_c);
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
		break;
	case 'b':
		flag_blind_effect = 1 - flag_blind_effect;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_blind_effect, flag_blind_effect);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 'g':
		color_effect = 1 - color_effect;
		glutPostRedisplay();
		break;
	case 'f':
		flag_fog = 1 - flag_fog;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_flag_fog, flag_fog);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	}
}

int prevy;
int clicked;
void motion(int x, int y) {
	if (!camera.move) return;
	if (!clicked) return;

	renew_cam_orientation_rotation_around_axis(prevy - y);

	prevy = y;

	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
	if ((button == GLUT_LEFT_BUTTON)) {
		if (state == GLUT_DOWN) {
			clicked = 1;
			prevy = y;
		}
		else if (state == GLUT_UP) clicked = 0;
	}
}

void reshape(int width, int height) {
	glViewport(0, 0, width, height);
	
	window_width = width; window_height = height;
	camera.aspect_ratio = (float)width / height;

	ProjectionMatrix = glm::perspective(TO_RADIAN * camera.fovy, camera.aspect_ratio, camera.near_c, camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	glutPostRedisplay();

}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO); 
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &rectangle_VAO);
	glDeleteBuffers(1, &rectangle_VBO);

	glDeleteVertexArrays(1, &tiger_VAO);
	glDeleteBuffers(1, &tiger_VBO);

	glDeleteTextures(N_TEXTURES_USED, texture_names);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene, 0);
	glutCloseFunc(cleanup);
}

void prepare_shader_program(void) {
	int i;
	char string[256];
	ShaderInfo shader_info_simple[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};
	ShaderInfo shader_info_TXPS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Phong_Tx.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Phong_Tx.frag" },
		{ GL_NONE, NULL }
	};
	ShaderInfo shader_info_GS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Gouraud.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Gouraud.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_simple = LoadShaders(shader_info_simple);
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");
	loc_ModelViewProjectionMatrix_simple = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");


	h_ShaderProgram_TXPS = LoadShaders(shader_info_TXPS);
	loc_ModelViewProjectionMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_global_ambient_color");

	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_TXPS, string);
	}

	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_exponent");

	loc_texture = glGetUniformLocation(h_ShaderProgram_TXPS, "u_base_texture");

	loc_screen_effect = glGetUniformLocation(h_ShaderProgram_TXPS, "screen_effect");
	loc_screen_frequency = glGetUniformLocation(h_ShaderProgram_TXPS, "screen_frequency");
	loc_screen_width = glGetUniformLocation(h_ShaderProgram_TXPS, "screen_width");
	loc_blind_effect = glGetUniformLocation(h_ShaderProgram_TXPS, "u_blind_effect");
	loc_blind_num = glGetUniformLocation(h_ShaderProgram_TXPS, "u_blind_num");
	loc_u_fragment_alpha = glGetUniformLocation(h_ShaderProgram_TXPS, "u_fragment_alpha");
	loc_u_flag_blending = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_blending");
	
	loc_flag_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_texture_mapping");
	loc_flag_fog = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_fog");

	h_ShaderProgram_GS = LoadShaders(shader_info_GS);
	loc_ModelViewProjectionMatrix_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_global_ambient_color");
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light_GS[i].light_on = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light_GS[i].position = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light_GS[i].ambient_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light_GS[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light_GS[i].specular_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light_GS[i].spot_direction = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light_GS[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light_GS[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light_GS[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_GS, string);
		}
	loc_material_GS.ambient_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.ambient_color");
	loc_material_GS.diffuse_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.diffuse_color");
	loc_material_GS.specular_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.specular_color");
	loc_material_GS.emissive_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.emissive_color");
	loc_material_GS.specular_exponent = glGetUniformLocation(h_ShaderProgram_GS, "u_material.specular_exponent");
}

void initialize_lights_and_material(void) { // follow OpenGL conventions for initialization
	int i;

	glUseProgram(h_ShaderProgram_TXPS);

	glUniform4f(loc_global_ambient_color, 0.9f, 0.9f, 0.9f, 1.0f);

	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		light[i].light_on = 1;
		glUniform1i(loc_light[i].light_on, 1); // turn off all lights initially
		glUniform4f(loc_light[i].position, 0.0f, 0.0f, 1.0f, 0.0f);
		glUniform4f(loc_light[i].ambient_color, 0.0f, 0.0f, 0.0f, 1.0f);
		if (i == 0) {
			glUniform4f(loc_light[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		else {
			glUniform4f(loc_light[i].diffuse_color, 0.0f, 0.0f, 0.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
		}
		glUniform3f(loc_light[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light[i].spot_exponent, 0.0f); // [0.0, 128.0]
		glUniform1f(loc_light[i].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light[i].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation
	}

	glUniform4f(loc_material.ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(loc_material.diffuse_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(loc_material.specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1f(loc_material.specular_exponent, 0.0f); // [0.0, 128.0]
	glUseProgram(0);

	glUseProgram(h_ShaderProgram_GS);
	glUniform4f(loc_global_ambient_color_GS, 0.1f, 0.1f, 0.1f, 1.0f);
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		glUniform1i(loc_light_GS[i].light_on, 1); // turn off all lights initially
		glUniform4f(loc_light_GS[i].position, 0.0f, 0.0f, 1.0f, 0.0f);
		glUniform4f(loc_light_GS[i].ambient_color, 0.0f, 0.0f, 0.0f, 1.0f);
		if (i == 0) {
			glUniform4f(loc_light_GS[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
			glUniform4f(loc_light_GS[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		else {
			glUniform4f(loc_light_GS[i].diffuse_color, 0.0f, 0.0f, 0.0f, 1.0f);
			glUniform4f(loc_light_GS[i].specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
		}
		glUniform3f(loc_light_GS[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light_GS[i].spot_exponent, 0.0f); // [0.0, 128.0]
		glUniform1f(loc_light_GS[i].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light_GS[i].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation
	}
	glUniform4f(loc_material_GS.ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(loc_material_GS.diffuse_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(loc_material_GS.specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4f(loc_material_GS.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1f(loc_material_GS.specular_exponent, 0.0f);
	glUseProgram(0);
}

void initialize_flags(void) {
	flag_tiger_animation = 1;
	flag_polygon_fill = 1;
	flag_texture_mapping = 1;
	flag_fog = 0;

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_flag_fog, flag_fog);
	glUniform1i(loc_flag_texture_mapping, flag_texture_mapping);
	glUseProgram(0);

}

void initialize_OpenGL(void) {
  	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//ViewMatrix = glm::lookAt(PRP_distance_scale[0] * glm::vec3(500.0f, 300.0f, 500.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	initialize_lights_and_material();
	initialize_flags();

	glGenTextures(N_TEXTURES_USED, texture_names);
}

void set_up_scene_lights(void) {
	// point_light_EC: use light 0
	//light[0].light_on = 1;
	light[0].position[0] = 0.0f; light[0].position[1] = 1000.0f; 	// point light position in EC
	light[0].position[2] = 0.0f; light[0].position[3] = 1.0f;

	light[0].ambient_color[0] = 0.4f; light[0].ambient_color[1] = 0.4f;
	light[0].ambient_color[2] = 0.4f; light[0].ambient_color[3] = 1.0f;

	light[0].diffuse_color[0] = 0.7f; light[0].diffuse_color[1] = 0.7f;
	light[0].diffuse_color[2] = 0.7f; light[0].diffuse_color[3] = 1.5f;

	light[0].specular_color[0] = 0.9f; light[0].specular_color[1] = 0.9f;
	light[0].specular_color[2] = 0.9f; light[0].specular_color[3] = 1.0f;

	// spot_light_WC: use light 1
	//light[1].light_on = 1;
	light[1].position[0] = -250.0f; light[1].position[1] = 500.0f; // spot light position in WC
	light[1].position[2] = 250.0f; light[1].position[3] = 1.0f;

	light[1].ambient_color[0] = 0.152f; light[1].ambient_color[1] = 0.152f;
	light[1].ambient_color[2] = 0.152f; light[1].ambient_color[3] = 1.0f;

	light[1].diffuse_color[0] = 0.572f; light[1].diffuse_color[1] = 0.572f;
	light[1].diffuse_color[2] = 0.572f; light[1].diffuse_color[3] = 1.0f;

	light[1].specular_color[0] = 0.772f; light[1].specular_color[1] = 0.772f;
	light[1].specular_color[2] = 0.772f; light[1].specular_color[3] = 1.0f;

	light[1].spot_direction[0] = sqrt(1.0f / 3.0f); light[1].spot_direction[1] = -sqrt(1.0f / 3.0f); // spot light direction in WC
	light[1].spot_direction[2] = -sqrt(1.0f / 3.0f);
	light[1].spot_cutoff_angle = 10.0f;
	light[1].spot_exponent = 8.0f;

	// spot_light_WC: use light 
	//light[2].light_on = 1;
	light[2].position[0] = camera.pos[0]; light[2].position[1] = camera.pos[1]; // spot light position in WC
	light[2].position[2] = camera.pos[2]; light[2].position[3] = 1.0f;

	light[2].ambient_color[0] = 0.2f; light[2].ambient_color[1] = 0.2f;
	light[2].ambient_color[2] = 0.152f; light[2].ambient_color[3] = 1.0f;

	light[2].diffuse_color[0] = 0.772f; light[2].diffuse_color[1] = 0.772f;
	light[2].diffuse_color[2] = 0.472f; light[2].diffuse_color[3] = 1.0f;

	light[2].specular_color[0] = 0.872f; light[2].specular_color[1] = 0.872f;
	light[2].specular_color[2] = 0.672f; light[2].specular_color[3] = 1.0f;

	light[2].spot_direction[0] = -camera.naxis[0]; light[2].spot_direction[1] = -camera.naxis[1]; // spot light direction in WC
	light[2].spot_direction[2] = -camera.naxis[2];

	light[2].spot_cutoff_angle = 10.0f;
	light[2].spot_exponent = 20.0f;

	// spot_light_WC: use light 
	//light[2].light_on = 1;
	glm::vec4 posi = position_dragon * glm::vec4(0, 0, 0, 1);
	light[3].position[0] = posi[0]; light[3].position[1] = posi[1]; // spot light position in WC
	light[3].position[2] = posi[2]; light[3].position[3] = 1.0f;

	light[3].ambient_color[0] = 0.2f; light[3].ambient_color[1] = 0.2f;
	light[3].ambient_color[2] = 0.152f; light[3].ambient_color[3] = 1.0f;

	light[3].diffuse_color[0] = 1; light[3].diffuse_color[1] = 0;
	light[3].diffuse_color[2] = 0; light[3].diffuse_color[3] = 1.0f;

	light[3].specular_color[0] = 1; light[3].specular_color[1] = 0;
	light[3].specular_color[2] = 0; light[3].specular_color[3] = 1.0f;

	light[3].spot_direction[0] = 0; light[3].spot_direction[1] = -1; // spot light direction in WC
	light[3].spot_direction[2] = 0;

	light[3].spot_cutoff_angle = 5.0f;
	light[3].spot_exponent = 20.0f;

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_light[0].light_on, light[0].light_on);
	glUniform4fv(loc_light[0].position, 1, light[0].position);
	glUniform4fv(loc_light[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light[0].specular_color, 1, light[0].specular_color);

	glUniform1i(loc_light[1].light_on, light[1].light_on);
	glm::vec4 position_EC = ViewMatrix * glm::vec4(light[1].position[0], light[1].position[1], light[1].position[2], light[1].position[3]);
	glUniform4fv(loc_light[1].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[1].ambient_color, 1, light[1].ambient_color);
	glUniform4fv(loc_light[1].diffuse_color, 1, light[1].diffuse_color);
	glUniform4fv(loc_light[1].specular_color, 1, light[1].specular_color);
	glm::vec3 direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[1].spot_direction[0], light[1].spot_direction[1],
		light[1].spot_direction[2]);
	glUniform3fv(loc_light[1].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light[1].spot_cutoff_angle, light[1].spot_cutoff_angle);
	glUniform1f(loc_light[1].spot_exponent, light[1].spot_exponent);

	glUniform1i(loc_light[2].light_on, light[2].light_on);
	position_EC = ViewMatrix * glm::vec4(light[2].position[0], light[2].position[1], light[2].position[2], light[2].position[3]);
	glUniform4fv(loc_light[2].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[2].ambient_color, 1, light[2].ambient_color);
	glUniform4fv(loc_light[2].diffuse_color, 1, light[2].diffuse_color);
	glUniform4fv(loc_light[2].specular_color, 1, light[2].specular_color);
	direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[2].spot_direction[0], light[2].spot_direction[1], light[2].spot_direction[2]);
	glUniform3fv(loc_light[2].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light[2].spot_cutoff_angle, light[2].spot_cutoff_angle);
	glUniform1f(loc_light[2].spot_exponent, light[2].spot_exponent);

	glUniform1i(loc_light[3].light_on, light[3].light_on);
	position_EC = ViewMatrix * glm::vec4(light[3].position[0], light[3].position[1], light[3].position[2], light[3].position[3]);
	glUniform4fv(loc_light[3].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[3].ambient_color, 1, light[3].ambient_color);
	glUniform4fv(loc_light[3].diffuse_color, 1, light[3].diffuse_color);
	glUniform4fv(loc_light[3].specular_color, 1, light[3].specular_color);
	direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[2].spot_direction[0], light[3].spot_direction[1], light[3].spot_direction[2]);
	glUniform3fv(loc_light[3].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light[3].spot_cutoff_angle, light[3].spot_cutoff_angle);
	glUniform1f(loc_light[3].spot_exponent, light[3].spot_exponent);
	glUseProgram(0);


	glUseProgram(h_ShaderProgram_GS);
	glUniform1i(loc_light_GS[0].light_on, light[0].light_on);
	glUniform4fv(loc_light_GS[0].position, 1, light[0].position);
	glUniform4fv(loc_light_GS[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light_GS[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light_GS[0].specular_color, 1, light[0].specular_color);

	glUniform1i(loc_light[1].light_on, light[1].light_on);
	// need to supply position in EC for shading
	position_EC = ViewMatrix * glm::vec4(light[1].position[0], light[1].position[1],
		light[1].position[2], light[1].position[3]);
	glUniform4fv(loc_light_GS[1].position, 1, &position_EC[0]);
	glUniform4fv(loc_light_GS[1].ambient_color, 1, light[1].ambient_color);
	glUniform4fv(loc_light_GS[1].diffuse_color, 1, light[1].diffuse_color);
	glUniform4fv(loc_light_GS[1].specular_color, 1, light[1].specular_color);
	// need to supply direction in EC for shading in this example shader
	direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[1].spot_direction[0], light[1].spot_direction[1], light[1].spot_direction[2]);
	glUniform3fv(loc_light_GS[1].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light_GS[1].spot_cutoff_angle, light[1].spot_cutoff_angle);
	glUniform1f(loc_light_GS[1].spot_exponent, light[1].spot_exponent);

	glUniform1i(loc_light_GS[2].light_on, light[2].light_on);
	position_EC = ViewMatrix * glm::vec4(light[2].position[0], light[2].position[1], light[2].position[2], light[2].position[3]);
	glUniform4fv(loc_light_GS[2].position, 1, &position_EC[0]);
	glUniform4fv(loc_light_GS[2].ambient_color, 1, light[2].ambient_color);
	glUniform4fv(loc_light_GS[2].diffuse_color, 1, light[2].diffuse_color);
	glUniform4fv(loc_light_GS[2].specular_color, 1, light[2].specular_color);
	direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[2].spot_direction[0], light[2].spot_direction[1], light[2].spot_direction[2]);
	glUniform3fv(loc_light_GS[2].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light_GS[2].spot_cutoff_angle, light[2].spot_cutoff_angle);
	glUniform1f(loc_light_GS[2].spot_exponent, light[2].spot_exponent);

	glUniform1i(loc_light_GS[3].light_on, light[3].light_on);
	position_EC = ViewMatrix * glm::vec4(light[3].position[0], light[3].position[1], light[3].position[2], light[3].position[3]);
	glUniform4fv(loc_light_GS[3].position, 1, &position_EC[0]);
	glUniform4fv(loc_light_GS[3].ambient_color, 1, light[3].ambient_color);
	glUniform4fv(loc_light_GS[3].diffuse_color, 1, light[3].diffuse_color);
	glUniform4fv(loc_light_GS[3].specular_color, 1, light[3].specular_color);
	direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[2].spot_direction[0], light[3].spot_direction[1], light[3].spot_direction[2]);
	glUniform3fv(loc_light_GS[3].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light_GS[3].spot_cutoff_angle, light[3].spot_cutoff_angle);
	glUniform1f(loc_light_GS[3].spot_exponent, light[3].spot_exponent);
	glUseProgram(0);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_spider();
	prepare_cow();
	prepare_dragon();
	prepare_godzilla();
	prepare_ironman();
	prepare_tank();
	prepare_bus();
	prepare_optimus();
	prepare_floor();
	set_up_scene_lights();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_camera();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 1
void main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 5.4.Tiger_Texture_PS_GLSL";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: '0', '1', 'a', 't', 'f', 'c', 'd', 'y', 'u', 'i', 'o', 'ESC'"  };

	glutInit(&argc, argv);
  	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(window_width, window_height);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
