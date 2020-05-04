/******************************************************************
 * Filename: choose_parking_space.cpp
 * Version: v1.0
 * Author: Meng Peng
 * Date: 2020-05-03
 * Description: choose parking space according to message from the 
 * topic parking_space
 * 
 ******************************************************************/

#include <string>
#include <sstream>
#include <ros/ros.h>
#include <std_msgs/String.h>

// global variable to update message from topic cmd_move
std::string cmd_move = "";

// callback funktion of subscriber
void callback_cmd_turn(const std_msgs::String::ConstPtr& msg)
{
    ROS_INFO("cmd_move: %s", msg->data.c_str());
    std::stringstream ss;
    ss << msg->data.c_str();
    ss >> cmd_move;
}

// function to check subscribed message from topic parking_space
void check_cmd_move(std::string cmd_str)
{
    ROS_INFO("check cmd_move: %s", cmd_str.c_str());
    if(cmd_str == "forward")
    {
        ROS_INFO("move forward");
        do_move('D');
    }
    if(cmd_str == "backward")
    {
        ROS_INFO("move backward");
        do_move('B');
    }
    if(cmd_str == "stop")
    {
        ROS_INFO("stop");
        do_move('P');
    }
}


int main(int argc, char **argv)
{
    ros::init(argc, argv, "choose_parking_space");
    ros::NodeHandle nh;
    ros::Subscriber sub = nh.subscribe<std_msgs::String>("cmd_move", 1, callback_cmd_turn);

    ros::Rate loop_rate(1);
    while (ros::ok())
    {
        ros::spinOnce();

        if (cmd_move != "")
        {
            check_cmd_move(cmd_move);
            cmd_move = "";
        }

        loop_rate.sleep();
    }

    return 0;
}