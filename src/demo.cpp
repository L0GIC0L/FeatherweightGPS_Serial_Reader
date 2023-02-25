// Demo:   demo.cpp
// Author: Evan Pezent (evanpezent.com)
// Date:   3/26/2021
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../implot/implot.h"
#include "../3rdparty/exprtk.hpp"
#include "../imgui/imgui_stdlib.h"
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "functions.h"
#include "plotfuncs.h"
#include <thread>
#include <chrono>
#include <atomic>

//********************************************************************************************

//Set the port directory (typically COM3 on windows)
//string port = "/dev/serial/by-id/usb-FTDI_FT230X_Basic_UART_D201170B-if00-port0";
string port = "/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_85734323430351B0C0A2-if00";

// Map to store the parsed data
std::multimap < std::string, std::string > parsed_data;

//Set safe file directory
std::string dir = R"(/home/nmiller/Projects/Brand New/FeatherweightGPS_Serial_Reader/src/)";

//Open the save file
std::ofstream csv_file ( dir + "data.csv" );

//Initialize input buffer
std::string line = "";

std::atomic<double> rs, rvx, rvy, rvz;


void read_and_parse()
{

  while ( true )
    {
      line = readSerialPort ( "GPS_STAT" );
      parseData ( parsed_data, line );
      rs.store (retrieveLatest ( parsed_data, R"(Seconds)" ));
      rvx.store (retrieveLatest ( parsed_data, R"(VelocityX)"));
      rvy.store (retrieveLatest( parsed_data, R"(VelocityY)"));
      rvz.store (retrieveLatest( parsed_data, R"(VelocityZ)"));
    }
}

int main ( int argc, char const *argv[] )
{
  if ( openSerialPort ( port ) )
    {
      // Initialize GLFW and create a window
      glfwInit();
      GLFWwindow* window = glfwCreateWindow ( 1280, 720, "ImGui Window", NULL, NULL );
      glfwMakeContextCurrent ( window );

      // Initialize GLEW
      glewExperimental = GL_TRUE;
      glewInit();

      // Setup ImGui context
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImPlot::CreateContext();
      ImGuiIO& io = ImGui::GetIO();
      ( void ) io;
      ImGui::StyleColorsDark();

      // Setup ImGui GLFW backend
      ImGui_ImplGlfw_InitForOpenGL ( window, true );
      ImGui_ImplOpenGL3_Init ( "#version 330" );

      std::thread t ( &read_and_parse );

      // Main loop
      while ( !glfwWindowShouldClose ( window ) )
        {
          glfwPollEvents();

          // Start ImGui frame
          ImGui_ImplOpenGL3_NewFrame();
          ImGui_ImplGlfw_NewFrame();
          ImGui::NewFrame();

          // Create a basic ImGui window
          ImGui::Begin ( "Hello, world!" );


          // retrieve the received value from the thread

          Demo_RealtimePlots ( rs.load(), rvx.load(), rvy.load(), rvz.load() );
          ImGui::End();
          ImGui::Begin ( "Hello, Altitude" );
          Demo_InfiniteLines();
          ImGui::End();

          ImGui::Begin ( "Map",0 );
          TileManager mngr;
          Demo_Map ( mngr );
          ImGui::End();

          // Render ImGui frame
          ImGui::Render();
          glClearColor ( 0.0f, 0.0f, 0.0f, 1.0f );
          glClear ( GL_COLOR_BUFFER_BIT );
          ImGui_ImplOpenGL3_RenderDrawData ( ImGui::GetDrawData() );

          glfwSwapBuffers ( window );
        }

      t.join();

      // Cleanup
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImPlot::CreateContext();
      ImGui::DestroyContext();
      glfwDestroyWindow ( window );
      glfwTerminate();
      return 0;
    }
}

