#include <iostream>
#include <k4a/k4a.h>
#include <k4arecord/playback.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cstring>

int main()
{
    ///////////////////// SETTINGS /////////////////////////
    // 1. create a folder callded "color" and one called "depth" in the directory of this script
    // 2. define what mkv recording should be used
    const char* recording = "record_5fps_1080p_NFOV_UNBINNED.mkv";
    ////////////////////////////////////////////////////////
    
    ///// setup for playback /////
    k4a_playback_t playback_handle = NULL;
    if (k4a_playback_open(recording, &playback_handle) != K4A_RESULT_SUCCEEDED)
    {
        printf("Failed to open recording\n");
        return 1;
    }

    k4a_calibration_t calib;
    if (k4a_playback_get_calibration(playback_handle, &calib) == K4A_RESULT_SUCCEEDED)
    {
        std::cout << "Successfully got calibration" << std::endl;
    }

    k4a_transformation_t transformation = k4a_transformation_create(&calib);
    if (transformation != NULL) {
        std::cout << "Successfully created transformation handle" << std::endl;
    }
    else {
        std::cout << "Failed to create tranformation handle" << std::endl;
    }

    k4a_capture_t capture = NULL;
    k4a_stream_result_t result = K4A_STREAM_RESULT_SUCCEEDED;
    ///// end setup /////

    int img_num = 0; 

    // start playback
    while (result == K4A_STREAM_RESULT_SUCCEEDED)
    {
        result = k4a_playback_get_next_capture(playback_handle, &capture);
        if (result == K4A_STREAM_RESULT_SUCCEEDED)
        {
            // can set image number like below for only extracing specific images, else all will be used
            //if (img_num > 50 and img_num < 100) { // use this if only want to extract images between frame 50 and 100
            if(true) {        

                // get color and depth images
                k4a_image_t color = k4a_capture_get_color_image(capture);
                k4a_image_t depth = k4a_capture_get_depth_image(capture);

                if (color != NULL) {                   
                    ////// transform depth image to color image frame /////
                    uint8_t* color_buffer = k4a_image_get_buffer(color);
                    int color_height = k4a_image_get_height_pixels(color);
                    int color_width = k4a_image_get_width_pixels(color);                
                    
                    k4a_image_t depth_transformed = NULL;
                    k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16, color_width, color_height, color_width * (int)sizeof(uint16_t), &depth_transformed);

                    if (k4a_transformation_depth_image_to_color_camera(transformation, depth, depth_transformed) != K4A_RESULT_SUCCEEDED) {
                        std::cout << "Failed transforming depth image" << std::endl;
                    }          
                    
                    ///// write color and transformed depth image /////
                    int depth_t_height = k4a_image_get_height_pixels(depth_transformed);
                    int depth_t_width = k4a_image_get_width_pixels(depth_transformed);             

                    size_t color_buffer_size = k4a_image_get_size(color);
                    std::string color_file_name = "color/"+std::to_string(img_num)+".png";

                    std::ofstream color_file_object(color_file_name, std::ios::out | std::ios::binary);
                    color_file_object.write(reinterpret_cast<char*>(color_buffer),color_buffer_size);
                    color_file_object.close();
                 
                    uint8_t* depth_buffer = k4a_image_get_buffer(depth_transformed);
                    cv::Mat depth_img(depth_t_height, depth_t_width, CV_16U, (void*)depth_buffer, cv::Mat::AUTO_STEP);

                    std::string depth_file_name = "depth/" + std::to_string(img_num) + ".png";
                    cv::imwrite(depth_file_name, depth_img);

                    std::cout << "Wrote image number: "<< img_num << std::endl;
                }
                else {
                    std::cout << "Color image is NULL" << std::endl;
                }
                              
            }
            img_num++;

            k4a_capture_release(capture);
        }
        else if (result == K4A_STREAM_RESULT_EOF)
        {
            // End of recording file reached
            break;
        }
    }

    k4a_transformation_destroy(transformation);

    if (result == K4A_STREAM_RESULT_FAILED)
    {
        printf("Failed to read entire recording\n");
        return 1;
    }
    else {
        std::cout << "Successfully got stream results" << std::endl;
    }

    k4a_playback_close(playback_handle);
    std::cout << "Finish!\n";
}
