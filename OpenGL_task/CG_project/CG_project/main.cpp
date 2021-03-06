#include <iostream>
#include <glew.h>
#include <glfw3.h>

#include "glm/vec3.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "shaders.h"
#include "models.h"
#include "renders.h"

class myCamera{
public:
    GLfloat delta_time;
    GLfloat last_time;
    GLfloat yaw;
    GLfloat pitch;
    GLfloat last_x;
    GLfloat last_y;
    bool first_mouse;
    float screen_width, screen_height;
    bool keystate[4]; // for key    A W D S
    glm::vec3 camera_pos;
    glm::vec3 camera_front;
    glm::vec3 camera_up;
}camera;
bool post_effect;
bool paralax;
#include "glfwwin.h"


int main(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    camera.screen_width = 1200;
    camera.screen_height = 800;
    GLFWwindow* window = glfwCreateWindow(camera.screen_width, camera.screen_height, "OpenGLWindow", nullptr, nullptr);
    if (window == nullptr){
	    std::cout << " Error create GLFW window " << std::endl;
	    glfwTerminate();
	    return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);  
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();  //glGetError();
    if (err != GLEW_OK){
        std::cout << " Error glewInit " << glGetError() << std::endl;
        return 0;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
camera.yaw   = -90.0f;
camera.pitch = 0.0f;
camera.last_x = 400;
camera.last_y = 300;
camera.camera_pos   = glm::vec3(3.2f, 1.0f,  3.0f);
camera.camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
camera.camera_up    = glm::vec3(0.0f, 1.0f,  0.0f);
camera.first_mouse = true;
camera.last_time = glfwGetTime();
camera.delta_time = 0;
for(int i = 0; i < 4; i++){
    camera.keystate[i] = false;
}
    int wwidth, wheight;
    glfwGetFramebufferSize(window, &wwidth, &wheight);
    glViewport(0, 0, wwidth, wheight);
/***************************************SHADERS*************************************/
    GLuint light_shad = comp_shader("./shaders/cb_light_v.c", "./shaders/cb_light_f.c");
    GLuint light_vao = my_set_light_vao();

    GLuint vao_skybox = set_skybox_vao();
    GLuint tex_skybox = set_skybox_tex();
    GLuint sh_skybox = comp_shader("./shaders/skybox_v.c", "./shaders/skybox_f.c");

    GLuint mirr_cube_shad = comp_shader("./shaders/mirror_cube_v.c", "./shaders/mirror_cube_f.c");
    GLuint mirr_cube_vao = set_mirrorcube_vao();
    
    GLuint b_texture = bilboard_texture("./textures/bilboard.png");
    GLuint b_vao =  bilboard_setVAO();
    GLuint b_sh = comp_shader("./shaders/bilboard_plane_v.c", "./shaders/bilboard_plane_f.c");

    post_effect = false;
    GLuint post_texture, post_buffer;
    set_p_buffer(post_buffer, post_texture, camera.screen_width, camera.screen_height);
    GLuint post_vao =  set_p_vao();
    GLuint post_shad = comp_shader("./shaders/post_effect_v.c", "./shaders/post_effect_f.c");
    

    Material mymaterial;
    mymaterial.shin = 0.2500f * 128.0f;
    load_textures(mymaterial);
    GLuint vao_for_plane = set_plane_vao1();
    GLuint cubes_shad = comp_shader("./shaders/cubes_v.c", "./shaders/cubes_f.c");
    GLuint vao_cubes = set_cubes_vao1();

    GLuint cubes_shadow_shad = comp_shader("./shaders/cubes_shadow_v.c", "./shaders/cubes_shadow_f.c");
    GLuint buffer_for_shadow;
    shadow_buffer(buffer_for_shadow, mymaterial.texture_for_shadow, camera.screen_width, camera.screen_height);
    paralax = false;

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        my_camera_step();
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glViewport(0, 0, camera.screen_width, camera.screen_height);
        //
        glm::mat4 model(1.0f);
        glm::mat4 view = glm::lookAt(camera.camera_pos, camera.camera_pos + camera.camera_front, camera.camera_up);
        glm::mat4 projection = glm::perspective(0.78539f, camera.screen_width / camera.screen_height, 0.1f, 100.0f);
        glm::vec3 my_light_position = glm::vec3(1.0f, 4.0f, 1 + 4.0f*glm::sin(2.0f*glfwGetTime()));



        glBindFramebuffer(GL_FRAMEBUFFER, buffer_for_shadow);
        glViewport(0, 0, camera.screen_width, camera.screen_height);
        glClear(GL_DEPTH_BUFFER_BIT);
        float l = 1.0f, r = 15.0f;
        glm::mat4 light_projection = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, l, r);
        glm::mat4 light_view = glm::lookAt(my_light_position, glm::vec3(-2.0f, 0.0f, 0.3f), glm::normalize(glm::vec3( 0.0f, 1.0f,  0.0f)));
        glm::mat4 shadow_m = light_projection * light_view;
        mymaterial.matrix_for_shadow = shadow_m;
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        for(int i = 0; i < 3; i++){
            glm::mat4 model_cubes = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f - 0.5f*i  , 1.5f - 0.1*i , 2.0f - 1.5f*i));
            model_cubes = glm::rotate(model_cubes, -0.11f, glm::vec3(1.0f, 0.0f, 0.0f));
            model_cubes = glm::scale(model_cubes, glm::vec3(0.5f, 0.5f, 0.5f));
            drow_plane1(cubes_shadow_shad, vao_cubes, 36,  mymaterial, camera.camera_pos,  my_light_position, model_cubes, view, projection);
        }
        //glCullFace(GL_BACK); 
        glDisable(GL_CULL_FACE);
        model = glm::mat4(1.0f);
        drow_plane1(cubes_shadow_shad, vao_for_plane, 6,  mymaterial, camera.camera_pos,  my_light_position, model, view, projection);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, camera.screen_width, camera.screen_height);


        //
        if(post_effect){
            glBindFramebuffer(GL_FRAMEBUFFER, post_buffer);
            glViewport(0, 0, camera.screen_width, camera.screen_height);
            glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 
        }

        glm::mat4 model_l = glm::translate(glm::mat4(1.0f), my_light_position);
        model_l = glm::scale(model_l, glm::vec3(0.35f));
        my_drow_light(light_vao, light_shad, model_l, view, projection);

        drow_mirrorcube(mirr_cube_vao, mirr_cube_shad,  glm::translate(glm::mat4(1.0f), glm::vec3(-6.5f, 6.5f, 4.0f)),  view,  projection, camera.camera_pos, tex_skybox);


glEnable(GL_STENCIL_TEST);
glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  
glStencilFunc(GL_ALWAYS, 1, 0xFF); 
glStencilMask(0xFF);
        for(int i = 0; i < 3; i++){
            glm::mat4 model_cubes = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f - 0.5f*i  , 1.5f - 0.1*i , 2.0f - 1.5f*i));
            model_cubes = glm::rotate(model_cubes, -0.11f, glm::vec3(1.0f, 0.0f, 0.0f));
            model_cubes = glm::scale(model_cubes, glm::vec3(0.5f, 0.5f, 0.5f));
            drow_plane1(cubes_shad, vao_cubes, 36,  mymaterial, camera.camera_pos,  my_light_position, model_cubes, view, projection);
        }
glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
glStencilMask(0x00);
        for(int i = 0; i < 3; i++){
            glm::mat4 model_cubes = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f - 0.5f*i  , 1.5f - 0.1*i , 2.0f - 1.5f*i));
            model_cubes = glm::rotate(model_cubes, -0.11f, glm::vec3(1.0f, 0.0f, 0.0f));
            model_cubes = glm::scale(model_cubes, glm::vec3(0.5f, 0.5f, 0.5f));
            model_cubes = glm::scale(model_cubes, glm::vec3(1.05f));
            drow_plane1(cubes_shad, vao_cubes, 36,  mymaterial, camera.camera_pos,  my_light_position, model_cubes, view, projection, true);
        }
glStencilMask(0xFF);
glDisable(GL_STENCIL_TEST);

        mymaterial.paralax = paralax;
        drow_plane1(cubes_shad, vao_for_plane, 6,  mymaterial, camera.camera_pos,  my_light_position, model, view, projection);
        mymaterial.paralax = false;

        drow_skybox(vao_skybox,  sh_skybox, tex_skybox ,  model, view,  projection);
        drow_bilbords(  b_sh ,  b_vao,  b_texture,  view, projection,  camera.camera_pos );

        if(post_effect){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, camera.screen_width, camera.screen_height);
            drow_p_effect( post_shad,  post_texture,  post_vao);
        }
        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}



