#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Windows.h>

#include <iostream>

#include "Timer.h"
#include "opencv2/opencv.hpp"

#define MAX_IMAGE_NUMBER 2048

void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

// Shader sources
const GLchar* vertexSource = R"glsl(
    #version 330 core
    in vec2 position;
    in vec2 texcoord;
    out vec2 Texcoords;
    void main()
    {
        //Colors = color;
        Texcoords = texcoord;
        gl_Position = vec4(position, 0.0, 1.0);
    }
)glsl";
const GLchar* fragmentSource = R"glsl(
    #version 330 core
    in vec2 Texcoords;
    out vec4 outColor;
    uniform sampler2DArray textures;
    uniform int layer_id;
    uniform int shader_light;
    uniform int light_map_id;
    uniform float light_weight;

    void main()
    {
        vec4 pix_color = texture(textures, vec3(Texcoords.xy, layer_id));
        outColor = pix_color;
        //if(shader_light == 1)
        //{
        //   float temp_thres = 0.012;
        //    vec4 light_color = texture(textures, vec3(Texcoords.xy, light_map_id));
        //    
        //    if(light_color.r > temp_thres)
        //    {   
        //        outColor = pix_color * (1.0 - light_weight) + light_color * light_weight;
        //    }
        //    else
        //    {
        //        outColor = pix_color;
        //    }
        //    
        //}
        //else
        //{
        //    outColor = pix_color;
        //    //outColor = vec4(0.0,1.0,0.0,1.0);
        //}
        
    }

)glsl";

unsigned char* load_images_to_mem(std::string im_folder, int* im_height,
                                  int* im_width, int* im_num,
                                  long long* accum_img_size) {
  unsigned char* mem_pt;

  std::vector<std::string> filename;
  cv::glob(im_folder, filename, false);

  int image_num = filename.size();
  (*im_num) = image_num;

  std::cout << "Total image #: " << image_num << std::endl;

  cv::Mat* imgs = new cv::Mat[image_num];
  unsigned char** img_pt = new unsigned char*[image_num];
  int* img_height = new int[image_num];
  int* img_width = new int[image_num];
  int* img_block_size = new int[image_num];

  long long total_size = 0;

  for (int i = 0; i < image_num; i++) {
    std::cout << "load image: " << filename[i] << std::endl;
    imgs[i] = cv::imread(filename[i], cv::IMREAD_COLOR);
    cv::cvtColor(imgs[i], imgs[i], cv::COLOR_BGR2RGB);
    img_pt[i] = (unsigned char*)imgs[i].data;
    img_height[i] = imgs[i].rows;
    img_width[i] = imgs[i].cols;

    img_block_size[i] = img_height[i] * img_width[i] * 3;
    accum_img_size[i] = total_size;
    total_size += (long long)img_block_size[i];

    std::cout << "Accumulate image size for " << i
              << " is: " << accum_img_size[i] << std::endl;
  }

  std::cout << "Total size for image data: " << total_size << "." << std::endl;

  mem_pt = new unsigned char[total_size];

  for (int i = 0; i < image_num; i++) {
    memcpy(&mem_pt[accum_img_size[i]], img_pt[i], img_block_size[i]);
  }

  (*im_width) = imgs[0].cols;
  (*im_height) = imgs[0].rows;

  // release image data
  for (int i = 0; i < image_num; i++) imgs[i].release();

  return mem_pt;
}

int main(int argc, char* argv[]) {
  // initial parameters ...

  std::string seq_scene_folder;
  std::string inbetween_pattern_folder;
  std::string seq_end_im_folder;

  bool insert_frame_inbetween = false;
  bool insert_frame_end = false;

  int switch_frame_num = 1;
  int monitor_id = 0;

  bool im_folder_flag = false;

  for (int para_id = 1; para_id < argc; para_id += 2) {
    std::cout << para_id << " " << argv[para_id] << std::endl;

    if (strcmp(argv[para_id], "-scene_path") == 0) {
      seq_scene_folder = argv[para_id + 1];
      im_folder_flag = true;
      std::cout << "input scene path: " << seq_scene_folder << std::endl;

    } else if (strcmp(argv[para_id], "-inbetween_path") == 0) {
      inbetween_pattern_folder = argv[para_id + 1];
      insert_frame_inbetween = true;
      std::cout << "input inbetween pattern path: " << inbetween_pattern_folder
                << std::endl;
    } else if (strcmp(argv[para_id], "-end_image_path") == 0) {
      seq_end_im_folder = argv[para_id + 1];
      insert_frame_end = true;
      std::cout << "input end scene pattern path: " << seq_end_im_folder
                << std::endl;
    } else if (strcmp(argv[para_id], "-segment_num") == 0) {
      switch_frame_num = atoi(argv[para_id + 1]);
    } else if (strcmp(argv[para_id], "-monitor_id") == 0) {
      monitor_id = atoi(argv[para_id + 1]);
      std::cout << "display on monitor: " << monitor_id << std::endl;
    }
  }

  if (im_folder_flag == false) {
    std::cout
        << "Usage: xxx.exe -scene_path im_path (-lit_map_pattern light_path "
           "-refresh_rate # -monitor_id # -light_weight wei_val)"
        << std::endl;
    std::cout << "Example: xxx.exe -scene_path data//*.png" << std::endl;
    std::cout << "Example: xxx.exe -scene_path data//*.png -inbetween_path "
                 "inbetween_data//*.png"
              << std::endl;
    std::cout << "Example: xxx.exe -scene_path data//*.png -inbetween_path "
                 "inbetween_data//*.png -segment_num 5"
              << std::endl;
    std::cout << "Example: xxx.exe -scene_path data//*.png -inbetween_path "
                 "inbetween_data//*.png -segment_num 5 -monitor_id 2"
              << std::endl;
    std::cout << "Example: xxx.exe -scene_path data//*.png -end_image_path "
                 "end_image_path//*.png -segment_num 5 -monitor_id 0"
              << std::endl;
    return (-1);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Load image files
  unsigned char* image_block = NULL;
  int img_width;
  int img_height;
  int img_num = 0;
  long long* img_size_accum = new long long[MAX_IMAGE_NUMBER];

  image_block = load_images_to_mem(seq_scene_folder, &img_height, &img_width,
                                   &img_num, img_size_accum);

  ////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Add inbetween in shader
  // Load inbetween image files
  unsigned char* inbetween_image_block = NULL;
  int inbetween_img_height;
  int inbetween_img_width;
  int inbetween_img_num = 0;
  long long* inbetween_img_size_accum = new long long[MAX_IMAGE_NUMBER];
  if (insert_frame_inbetween) {
    inbetween_image_block = load_images_to_mem(
        inbetween_pattern_folder, &inbetween_img_height, &inbetween_img_width,
        &inbetween_img_num, inbetween_img_size_accum);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Add end sequence images in shader
  // Load end sequence image files
  unsigned char* end_image_block = NULL;
  int end_img_height;
  int end_img_width;
  int end_img_num = 0;
  long long* end_img_size_accum = new long long[MAX_IMAGE_NUMBER];
  if (insert_frame_end) {
    end_image_block =
        load_images_to_mem(seq_end_im_folder, &end_img_height, &end_img_width,
                           &end_img_num, end_img_size_accum);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Calculate sequence pattern information
  int seg_frame;
  if (switch_frame_num > 1 || insert_frame_inbetween)
    seg_frame = switch_frame_num * (inbetween_img_num + 2);
  else
    seg_frame = 1;

  int total_frame = seg_frame * img_num + end_img_num;
  int total_im_num = img_num + inbetween_img_num + end_img_num;

  // Use timer to calculate frame rate
  CpuTimer timerCPU;

  // GL window initial
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  int monitor_count;
  GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);

  int width_mm, height_mm;
  glfwGetMonitorPhysicalSize(monitors[monitor_id], &width_mm, &height_mm);

  int xpos, ypos, xwidth, xheight;
  glfwGetMonitorWorkarea(monitors[monitor_id], &xpos, &ypos, &xwidth, &xheight);

  std::cout << "monitor height: " << xheight << ", monitor width: " << xwidth
            << std::endl;

  // GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL", nullptr,
  // nullptr); // Windowed
  GLFWwindow* window = glfwCreateWindow(
      xwidth, xheight, "OpenGL", monitors[monitor_id], nullptr);  // Fullscreen
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  glfwSwapInterval(1);  // vsync open!!! Important!!!

  // Initialize GLEW
  glewExperimental = GL_TRUE;
  glewInit();

  //// The max size of texture we can use

  // Create Vertex Array Object
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Create a Vertex Buffer Object and copy the vertex data to it
  GLuint vbo;
  glGenBuffers(1, &vbo);

  GLfloat vertices[] = {
      //  Position      Texcoords
      -1.0f, 1.0f,  0.0f, 0.0f,  // Top-left
      1.0f,  1.0f,  1.0f, 0.0f,  // Top-right
      1.0f,  -1.0f, 1.0f, 1.0f,  // Bottom-right
      -1.0f, -1.0f, 0.0f, 1.0f   // Bottom-left
  };

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Create an element array
  GLuint ebo;
  glGenBuffers(1, &ebo);

  GLuint triangle_ind[] = {0, 1, 2, 2, 3, 0};

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangle_ind), triangle_ind,
               GL_STATIC_DRAW);

  // Create and compile the vertex shader
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
  glCompileShader(vertexShader);

  // Create and compile the fragment shader
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);

  // Link the vertex and fragment shader into a shader program
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glBindFragDataLocation(shaderProgram, 0, "outColor");
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);

  // Specify the layout of the vertex data
  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                        0);

  GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
  glEnableVertexAttribArray(texAttrib);
  glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                        (void*)(2 * sizeof(GLfloat)));

  // Load texture
  GLuint tex_array;
  glGenTextures(1, &tex_array);

  glTexImage3D(GL_TEXTURE_2D_ARRAY,
               0,                                    // level
               GL_RGB8,                              // Internal format
               img_width, img_height, total_im_num,  // width,height,depth
               0,
               GL_RGB,            // format
               GL_UNSIGNED_BYTE,  // type
               0);

  int im_start_id = 0;
  int im_end_id = 0;
  for (int i = 0; i < img_num; i++) {
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                    0,                         // Mipmap number
                    0, 0, i + im_start_id,     // xoffset, yoffset, zoffset
                    img_width, img_height, 1,  // width, height, depth
                    GL_RGB,                    // format
                    GL_UNSIGNED_BYTE,          // type
                    &(image_block[img_size_accum[i]]));  // pointer to data
  }
  im_end_id = img_num;
  if (insert_frame_inbetween) {
    im_start_id = im_end_id;
    for (int i = 0; i < inbetween_img_num; i++) {
      glTexSubImage3D(
          GL_TEXTURE_2D_ARRAY,
          0,                      // Mipmap number
          0, 0, i + im_start_id,  // xoffset, yoffset, zoffset
          inbetween_img_width, inbetween_img_height,
          1,                 // width, height, depth
          GL_RGB,            // format
          GL_UNSIGNED_BYTE,  // type
          &(inbetween_image_block[inbetween_img_size_accum[i]]));  // pointer to
                                                                   // data
    }
    im_end_id += inbetween_img_num;
  }
  if (insert_frame_end) {
    im_start_id = im_end_id;
    for (int i = 0; i < end_img_num; i++) {
      glTexSubImage3D(
          GL_TEXTURE_2D_ARRAY,
          0,                      // Mipmap number
          0, 0, i + im_start_id,  // xoffset, yoffset, zoffset
          end_img_width, end_img_height,
          1,                 // width, height, depth
          GL_RGB,            // format
          GL_UNSIGNED_BYTE,  // type
                      &(end_image_block[end_img_size_accum[i]]));  // pointer to
                                                                   // data
    }
  }

  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, tex_array);

  glUniform1i(glGetUniformLocation(shaderProgram, "textures"), tex_array);

  GLint myUniformLocationLayer =
      glGetUniformLocation(shaderProgram, "layer_id");

  double lastTime = glfwGetTime();
  int tex_id = 0;
  int frame_id = 0;

  while (!glfwWindowShouldClose(window)) {
    double currentTime = glfwGetTime();
    printf("%f ms/frame\n", (currentTime - lastTime) * 1000.0);
    lastTime = currentTime;

    // Handle Keyboard Input
    // -----
    processInput(window);

    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Locate Texture id (TODO!!!)
    int seg_id = frame_id / seg_frame;
    int in_seg_id = frame_id - seg_id * seg_frame;
    if (seg_id >= img_num) {
      // Show end sequence frame
      tex_id = img_num + inbetween_img_num + in_seg_id;
    } else {
      if (seg_frame == 1) {
        // Show image in sequence
        tex_id = seg_id;
      } else {
        if (insert_frame_inbetween) {
          int repeat_id = in_seg_id / (inbetween_img_num + 2);
          int pattern_id = in_seg_id - repeat_id * (inbetween_img_num + 2);
          if (pattern_id < inbetween_img_num) {
            // show inbetween frame
            tex_id = img_num + pattern_id;
          } else {
            printf("pattern id is %d\n", pattern_id);
            int img_id = pattern_id - inbetween_img_num;
            printf("iamge id is %d\n", img_id);
            // show image
            tex_id = seg_id + img_id;
          }
        } else {
          int repeat_id = in_seg_id / 2;
          int pattern_id = in_seg_id - repeat_id * 2;
          // show image
          tex_id = seg_id + pattern_id;
        }
      }
    }
    printf("texture id is %d\n", tex_id);
    glUniform1i(myUniformLocationLayer, tex_id);

    // Frame count
    frame_id++;
    if (frame_id == total_frame) {
      frame_id = 0;
    }

    // Draw a rectangle from the 2 triangles using 6 indices
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(window);
    glFinish();

    glfwPollEvents();
  }

  // Clean everything and exit program
  glDeleteTextures(1, &tex_array);

  glDeleteProgram(shaderProgram);
  glDeleteShader(fragmentShader);
  glDeleteShader(vertexShader);
  glDeleteBuffers(1, &ebo);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);

  glfwTerminate();

  // Release all loaded image data
  free(image_block);
  image_block = NULL;
  free(inbetween_image_block);
  inbetween_image_block = NULL;
  free(end_image_block);
  end_image_block = NULL;

  return 0;
}
