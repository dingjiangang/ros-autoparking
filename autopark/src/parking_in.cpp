/******************************************************************
 * Filename: parking_in.h
 * Version: v1.0
 * Author: Meng Peng
 * Date: 2020-05-11
 * Description: main program for parking into parking space inkl. 
 * functions for perpendicular and parallel parking
 * 
 ******************************************************************/

#include "autopark/autoparking.h"
#include "autopark/parking_in.h"

using namespace std;

static uint32_t choosed_parking_space = 0;  // choosed parking space
static bool trigger_check = false;          // flag to enable check function 
static bool trigger_spinner = false;        // flag to enable spinners

static bool parking_finished = false;        // flag of parking finished
static bool time_out = false;               // flag of time out

boost::shared_ptr<ros::AsyncSpinner> sp_spinner;   // create a shared_ptr for AsyncSpinner object


// callback from global callback queue
// callback of sub_parking_space
void callback_parking_space(const std_msgs::Header::ConstPtr& msg)
{
    ROS_INFO("call callback of parking_space: %d", msg->seq);
    choosed_parking_space = msg->seq;
}

// CONSTRUCTOR: called when this object is created to set up subscribers and publishers
ParkingIn::ParkingIn(ros::NodeHandle* nodehandle):nh_(*nodehandle)
{
    ROS_INFO("call constructor in perpendicular_parking_right");

    sub_car_speed_ = nh_.subscribe<std_msgs::Float32>("car_speed", 1, \
    &ParkingIn::callback_car_speed, this);

    sub_apa_lf_ = nh_.subscribe<sensor_msgs::Range>("apa_lf", 1, \
    &ParkingIn::callback_apa_lf, this);

    sub_apa_lb_ = nh_.subscribe<sensor_msgs::Range>("apa_lb", 1, \
    &ParkingIn::callback_apa_lb, this);

    sub_apa_lb2_ = nh_.subscribe<sensor_msgs::Range>("apa_lb2", 1, \
    &ParkingIn::callback_apa_lb2, this);

    sub_apa_rf_ = nh_.subscribe<sensor_msgs::Range>("apa_rf", 1, \
    &ParkingIn::callback_apa_rf, this);

    sub_apa_rb_ = nh_.subscribe<sensor_msgs::Range>("apa_rb", 1, \
    &ParkingIn::callback_apa_rb, this);

    sub_apa_rb2_ = nh_.subscribe<sensor_msgs::Range>("apa_rb2", 1, \
    &ParkingIn::callback_apa_rb2, this);

    sub_upa_fl_ = nh_.subscribe<sensor_msgs::Range>("upa_fl", 1, \
    &ParkingIn::callback_upa_fl, this);

    sub_upa_fcl_ = nh_.subscribe<sensor_msgs::Range>("upa_fcl", 1, \
    &ParkingIn::callback_upa_fcl, this);

    sub_upa_fcr_ = nh_.subscribe<sensor_msgs::Range>("upa_fcr", 1, \
    &ParkingIn::callback_upa_fcr, this);

    sub_upa_fr_ = nh_.subscribe<sensor_msgs::Range>("upa_fr", 1, \
    &ParkingIn::callback_upa_fr, this);

    sub_upa_bl_ = nh_.subscribe<sensor_msgs::Range>("upa_bl", 1, \
    &ParkingIn::callback_upa_bl, this);

    sub_upa_bcl_ = nh_.subscribe<sensor_msgs::Range>("upa_bcl", 1, \
    &ParkingIn::callback_upa_bcl, this);

    sub_upa_bcr_ = nh_.subscribe<sensor_msgs::Range>("upa_bcr", 1, \
    &ParkingIn::callback_upa_bcr, this);

    sub_upa_br_ = nh_.subscribe<sensor_msgs::Range>("upa_br", 1, \
    &ParkingIn::callback_upa_br, this);

    timer_ = nh_.createWallTimer(ros::WallDuration(parking_time), \
    &ParkingIn::callback_timer, this);

    pub_move_ = nh_.advertise<std_msgs::Float32>("cmd_move", 1);

    pub_turn_ = nh_.advertise<std_msgs::Char>("cmd_turn", 1);
}

// DESTRUCTOR: called when this object is deleted to release memory 
ParkingIn::~ParkingIn(void)
{
    ROS_INFO("call destructor in perpendicular_parking_right");
}

// callbacks from custom callback queue
// callback of sub_car_speed_
void ParkingIn::callback_car_speed(const std_msgs::Float32::ConstPtr& msg)
{
    ROS_INFO("call callback of car_speed: speed=%f", msg->data);
    msg_car_speed_.data = msg->data;
}

// callback of timer: parking should be finished in a certain time
void ParkingIn::callback_timer(const ros::WallTimerEvent& event)
{
    ROS_INFO("timer of %f[s] is triggered", parking_time);

    time_out = true;
}

// callback of sub_apa_lf_
void ParkingIn::callback_apa_lf(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of apa_lf: range=%f", msg->range);
    msg_apa_lf_.header.stamp = msg->header.stamp;
    msg_apa_lf_.header.frame_id = msg->header.frame_id;
    msg_apa_lf_.range = msg->range;
}

// callback of sub_apa_lb_
void ParkingIn::callback_apa_lb(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of apa_lb: range=%f", msg->range);
    msg_apa_lb_.header.stamp = msg->header.stamp;
    msg_apa_lb_.header.frame_id = msg->header.frame_id;
    msg_apa_lb_.range = msg->range;
}

// callback of sub_apa_lb2_
void ParkingIn::callback_apa_lb2(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of apa_lb2: range=%f", msg->range);
    msg_apa_lb2_.header.stamp = msg->header.stamp;
    msg_apa_lb2_.header.frame_id = msg->header.frame_id;
    msg_apa_lb2_.range = msg->range;
}

// callback of sub_apa_rf_
void ParkingIn::callback_apa_rf(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of apa_rf: range=%f", msg->range);
    msg_apa_rf_.header.stamp = msg->header.stamp;
    msg_apa_rf_.header.frame_id = msg->header.frame_id;
    msg_apa_rf_.range = msg->range;
}

// callback of sub_apa_rb_
void ParkingIn::callback_apa_rb(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of apa_rb: range=%f", msg->range);
    msg_apa_rb_.header.stamp = msg->header.stamp;
    msg_apa_rb_.header.frame_id = msg->header.frame_id;
    msg_apa_rb_.range = msg->range;
}

// callback of sub_apa_rb2_
void ParkingIn::callback_apa_rb2(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of apa_rb2: range=%f", msg->range);
    msg_apa_rb2_.header.stamp = msg->header.stamp;
    msg_apa_rb2_.header.frame_id = msg->header.frame_id;
    msg_apa_rb2_.range = msg->range;
}

// callback of sub_upa_fl_
void ParkingIn::callback_upa_fl(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of upa_fl: range=%f", msg->range);
    msg_upa_fl_.header.stamp = msg->header.stamp;
    msg_upa_fl_.header.frame_id = msg->header.frame_id;
    msg_upa_fl_.range = msg->range;
}

// callback of sub_upa_fcl_
void ParkingIn::callback_upa_fcl(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of upa_fcl: range=%f", msg->range);
    msg_upa_fcl_.header.stamp = msg->header.stamp;
    msg_upa_fcl_.header.frame_id = msg->header.frame_id;
    msg_upa_fcl_.range = msg->range;
}

// callback of sub_upa_fcr_
void ParkingIn::callback_upa_fcr(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of upa_fcr: range=%f", msg->range);
    msg_upa_fcr_.header.stamp = msg->header.stamp;
    msg_upa_fcr_.header.frame_id = msg->header.frame_id;
    msg_upa_fcr_.range = msg->range;
}

// callback of sub_upa_fr_
void ParkingIn::callback_upa_fr(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of upa_fr: range=%f", msg->range);
    msg_upa_fr_.header.stamp = msg->header.stamp;
    msg_upa_fr_.header.frame_id = msg->header.frame_id;
    msg_upa_fr_.range = msg->range;
}

// callback of sub_upa_bl_
void ParkingIn::callback_upa_bl(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of upa_bl: range=%f", msg->range);
    msg_upa_bl_.header.stamp = msg->header.stamp;
    msg_upa_bl_.header.frame_id = msg->header.frame_id;
    msg_upa_bl_.range = msg->range;
}

// callback of sub_upa_bcl_
void ParkingIn::callback_upa_bcl(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of upa_bcl: range=%f", msg->range);
    msg_upa_bcl_.header.stamp = msg->header.stamp;
    msg_upa_bcl_.header.frame_id = msg->header.frame_id;
    msg_upa_bcl_.range = msg->range;
}

// callback of sub_upa_bcr_
void ParkingIn::callback_upa_bcr(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of upa_bcr: range=%f", msg->range);
    msg_upa_bcr_.header.stamp = msg->header.stamp;
    msg_upa_bcr_.header.frame_id = msg->header.frame_id;
    msg_upa_bcr_.range = msg->range;
}

// callback of sub_upa_br_
void ParkingIn::callback_upa_br(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of upa_br: range=%f", msg->range);
    msg_upa_br_.header.stamp = msg->header.stamp;
    msg_upa_br_.header.frame_id = msg->header.frame_id;
    msg_upa_br_.range = msg->range;
}


// function of moving a given distance
void ParkingIn::move_before_parking(float move_distance)
{
    // move forward with speed_parking_forward until move_distacne is reached
    msg_cmd_move_.data = speed_parking_forward;
    pub_move_.publish(msg_cmd_move_);

    static float moved_distance = 0;
    static ros::Time move_start, move_end;
    static double move_duration;
    move_start = ros::Time::now();
    while (moved_distance < move_distance)
    {
        ros::Duration(0.01).sleep();
        move_end = ros::Time::now();
        move_duration = (move_end - move_start).toSec();
        moved_distance += msg_car_speed_.data * move_duration;
        move_start = move_end;
    }

    // stop
    msg_cmd_move_.data = 0;
    pub_move_.publish(msg_cmd_move_);
}


// *****************************************************
// function of perpendicular parking on the left side
// *****************************************************
void ParkingIn::perpendicular_parking_left()
{
    // stop
    msg_cmd_move_.data = 0;
    pub_move_.publish(msg_cmd_move_);
    // turn full right
    msg_cmd_turn_.data = 'L';
    pub_turn_.publish(msg_cmd_turn_);
    // move backward with speed_parking_backward
    msg_cmd_move_.data = speed_parking_backward;
    pub_move_.publish(msg_cmd_move_);

    ros::Rate looprate(20);

    // check and change car posture with a rate 20Hz until car rear is in parking space
    while (ros::ok())
    {
        // publish move and turn commands
        pub_move_.publish(msg_cmd_move_);
        pub_turn_.publish(msg_cmd_turn_);

        // car rear is already in parking space
        if (msg_apa_lb_.range < parking_distance_max && msg_apa_rb_.range < parking_distance_max)
        {
            break;      // break this loop
        }
        // car rear is still out of parking space
        else
        {
            // car is too close to the parkwall (car) on the left side
            if (msg_apa_lb_.range < parking_distance_min || msg_upa_bl_.range < parking_distance_min)
            {
                // turn straight
                msg_cmd_turn_.data = 'D';
                pub_turn_.publish(msg_cmd_turn_);
            }
            else
            {
                // keep turnning full left
                msg_cmd_turn_.data = 'L';
                pub_turn_.publish(msg_cmd_turn_);
            }
        }

        looprate.sleep();
    }

    // check and change car posture with a rate 20Hz until parking is finished
    while (ros::ok())
    {
        // publish move and turn commands
        pub_move_.publish(msg_cmd_move_);
        pub_turn_.publish(msg_cmd_turn_);

        // car is parallel to the left parkwall (car)
        if (fabs(msg_apa_lb_.range - msg_apa_lb2_.range) <= apa_tolerance)
        {
            // turn straight
            msg_cmd_turn_.data = 'D';
            pub_turn_.publish(msg_cmd_turn_);
            // keep moving backward with speed_parking_backward
            msg_cmd_move_.data = speed_parking_backward;
            pub_move_.publish(msg_cmd_move_);

            // car rear is close to the back parkwall, parking finish
            if (min(msg_upa_bcl_.range, msg_upa_bcr_.range) < parking_distance_min)
            {
                // stop
                msg_cmd_move_.data = 0;
                pub_move_.publish(msg_cmd_move_);

                parking_finished = true;     // parking finished!
                break;      // break this loop
            }
        }
        // car is not parallel to the right parkwall (car)
        else
        {
            // car moves forward
            if (msg_car_speed_.data > 0)
            {
                // car head is closer to the left parkwall (car) than car rear
                if ((msg_apa_lb_.range - msg_apa_lb2_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'R')
                    {
                        // turn a little right
                        msg_cmd_turn_.data = 'r';
                    }
                }
                // car rear is closer to the left parkwall (car) than car head
                else if ((msg_apa_lb2_.range - msg_apa_lb_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'L')
                    {
                        // turn a little left
                        msg_cmd_turn_.data = 'l';
                    }
                }
                else
                {
                    // do nothing
                }

                // car head is too close to the left parkwall (car)
                if (msg_apa_lf_.range < parking_distance_min)
                {
                    // keep the steering full right
                    msg_cmd_turn_.data = 'R';
                    pub_turn_.publish(msg_cmd_turn_);
                }
                // car head is too close to the right parkwall (car)
                if (msg_apa_rf_.range < parking_distance_min)
                {
                    // keep the steering full left
                    msg_cmd_turn_.data = 'L';
                    pub_turn_.publish(msg_cmd_turn_);
                }

                // car is getting out of parking space
                if ((msg_cmd_turn_.data == 'R' && msg_apa_lb_.range > parking_distance_max) \
                || (msg_cmd_turn_.data == 'L' && msg_apa_rb_.range > parking_distance_max))
                {
                    // stop
                    msg_cmd_move_.data = 0;
                    pub_move_.publish(msg_cmd_move_);
                    // turn straight
                    msg_cmd_turn_.data = 'D';
                    pub_turn_.publish(msg_cmd_turn_);
                    // move backward
                    msg_cmd_move_.data = speed_parking_backward;
                    pub_move_.publish(msg_cmd_move_);
                }
            }
            // car moves backward
            if (msg_car_speed_.data < 0)
            {
                // car head is closer to the left parkwall (car) than car rear
                if ((msg_apa_lb_.range - msg_apa_lb2_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'L')
                    {
                        // turn a little left
                        msg_cmd_turn_.data = 'l';
                    }
                }
                // car rear is closer to the left parkwall (car) than car head
                else if ((msg_apa_lb2_.range - msg_apa_lb_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'R')
                    {
                        // turn a little right
                        msg_cmd_turn_.data = 'r';
                    }
                }
                else
                {
                    // do nothing
                }

                // car rear is too close to the left parkwall (car)
                if (msg_apa_lb_.range < parking_distance_min)
                {
                    // stop
                    msg_cmd_move_.data = 0;
                    pub_move_.publish(msg_cmd_move_);
                    // turn full left
                    msg_cmd_turn_.data = 'L';
                    pub_turn_.publish(msg_cmd_turn_);
                    // move forward
                    msg_cmd_move_.data = speed_parking_forward;
                    pub_move_.publish(msg_cmd_move_);
                }
                // car rear is too close to the right parkwall (car)
                if (msg_apa_rf_.range < parking_distance_min)
                {
                    // stop
                    msg_cmd_move_.data = 0;
                    pub_move_.publish(msg_cmd_move_);
                    // keep the steering full right
                    msg_cmd_turn_.data = 'R';
                    pub_turn_.publish(msg_cmd_turn_);
                    // move forward
                    msg_cmd_move_.data = speed_parking_forward;
                    pub_move_.publish(msg_cmd_move_);
                }
            }
            // car stops
            else
            {
                // do nothing
            }
        }

        looprate.sleep();
    }
} 


// *****************************************************
// function of perpendicular parking on the right side
// *****************************************************
void ParkingIn::perpendicular_parking_right()
{
    // stop
    msg_cmd_move_.data = 0;
    pub_move_.publish(msg_cmd_move_);
    // turn full right
    msg_cmd_turn_.data = 'R';
    pub_turn_.publish(msg_cmd_turn_);
    // move backward with speed_parking_backward
    msg_cmd_move_.data = speed_parking_backward;
    pub_move_.publish(msg_cmd_move_);

    ros::Rate looprate(20);

    // check and change car posture with a rate 20Hz until car rear is in parking space
    while (ros::ok())
    {
        // publish move and turn commands
        pub_move_.publish(msg_cmd_move_);
        pub_turn_.publish(msg_cmd_turn_);

        // car rear is already in parking space
        if (msg_apa_lb_.range < parking_distance_max && msg_apa_rb_.range < parking_distance_max)
        {
            break;      // break this loop
        }
        // car rear is still out of parking space
        else
        {
            // car is too close to the parkwall (car) on the right side
            if (msg_apa_rb_.range < parking_distance_min || msg_upa_br_.range < parking_distance_min)
            {
                // turn straight
                msg_cmd_turn_.data = 'D';
                pub_turn_.publish(msg_cmd_turn_);
            }
            else
            {
                // keep turnning full right
                msg_cmd_turn_.data = 'R';
                pub_turn_.publish(msg_cmd_turn_);
            }
        }

        looprate.sleep();
    }

    // check and change car posture with a rate 20Hz until parking is finished
    while (ros::ok())
    {
        // publish move and turn commands
        pub_move_.publish(msg_cmd_move_);
        pub_turn_.publish(msg_cmd_turn_);

        // car is parallel to the right parkwall (car)
        if (fabs(msg_apa_rb_.range - msg_apa_rb2_.range) <= apa_tolerance)
        {
            // turn straight
            msg_cmd_turn_.data = 'D';
            pub_turn_.publish(msg_cmd_turn_);
            // keep moving backward with speed_parking_backward
            msg_cmd_move_.data = speed_parking_backward;
            pub_move_.publish(msg_cmd_move_);

            // car rear is close to the back parkwall, parking finish
            if (min(msg_upa_bcl_.range, msg_upa_bcr_.range) < parking_distance_min)
            {
                // stop
                msg_cmd_move_.data = 0;
                pub_move_.publish(msg_cmd_move_);

                parking_finished = true;     // parking finished!
                break;      // break this loop
            }
        }
        // car is not parallel to the right parkwall (car)
        else
        {
            // car moves forward
            if (msg_car_speed_.data > 0)
            {
                // car head is closer to the right parkwall (car) than car rear
                if ((msg_apa_rb_.range - msg_apa_rb2_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'L')
                    {
                        // turn a little left
                        msg_cmd_turn_.data = 'l';
                    }
                }
                // car rear is closer to the right parkwall (car) than car head
                else if ((msg_apa_rb2_.range - msg_apa_rb_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'R')
                    {
                        // turn a little right
                        msg_cmd_turn_.data = 'r';
                    }
                }
                else
                {
                    // do nothing
                }

                // car head is too close to the right parkwall (car)
                if (msg_apa_rf_.range < parking_distance_min)
                {
                    // keep the steering full left
                    msg_cmd_turn_.data = 'L';
                    pub_turn_.publish(msg_cmd_turn_);
                }
                // car head is too close to the left parkwall (car)
                if (msg_apa_lf_.range < parking_distance_min)
                {
                    // keep the steering full right
                    msg_cmd_turn_.data = 'R';
                    pub_turn_.publish(msg_cmd_turn_);
                }

                // car is getting out of parking space
                if ((msg_cmd_turn_.data == 'R' && msg_apa_lb_.range > parking_distance_max) \
                || (msg_cmd_turn_.data == 'L' && msg_apa_rb_.range > parking_distance_max))
                {
                    // stop
                    msg_cmd_move_.data = 0;
                    pub_move_.publish(msg_cmd_move_);
                    // turn straight
                    msg_cmd_turn_.data = 'D';
                    pub_turn_.publish(msg_cmd_turn_);
                    // move backward
                    msg_cmd_move_.data = speed_parking_backward;
                    pub_move_.publish(msg_cmd_move_);
                }
            }
            // car moves backward
            if (msg_car_speed_.data < 0)
            {
                // car head is closer to the right parkwall (car) than car rear
                if ((msg_apa_rb_.range - msg_apa_rb2_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'R')
                    {
                        // turn a little right
                        msg_cmd_turn_.data = 'r';
                    }
                }
                // car rear is closer to the right parkwall (car) than car head
                else if ((msg_apa_rb2_.range - msg_apa_rb_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'L')
                    {
                        // turn a little left
                        msg_cmd_turn_.data = 'l';
                    }
                }
                else
                {
                    // do nothing
                }

                // car rear is too close to the right parkwall (car)
                if (msg_apa_rb_.range < parking_distance_min)
                {
                    // stop
                    msg_cmd_move_.data = 0;
                    pub_move_.publish(msg_cmd_move_);
                    // turn full right
                    msg_cmd_turn_.data = 'R';
                    pub_turn_.publish(msg_cmd_turn_);
                    // move forward
                    msg_cmd_move_.data = speed_parking_forward;
                    pub_move_.publish(msg_cmd_move_);
                }
                // car rear is too close to the left parkwall (car)
                if (msg_apa_lf_.range < parking_distance_min)
                {
                    // stop
                    msg_cmd_move_.data = 0;
                    pub_move_.publish(msg_cmd_move_);
                    // keep the steering full left
                    msg_cmd_turn_.data = 'L';
                    pub_turn_.publish(msg_cmd_turn_);
                    // move forward
                    msg_cmd_move_.data = speed_parking_forward;
                    pub_move_.publish(msg_cmd_move_);
                }
            }
            // car stops
            else
            {
                // do nothing
            }
        }

        looprate.sleep();
    }
}


// *****************************************************
// function of parallel parking on the left side
// *****************************************************
void ParkingIn::parallel_parking_left()
{
    // stop
    msg_cmd_move_.data = 0;
    pub_move_.publish(msg_cmd_move_);
    // turn full left
    msg_cmd_turn_.data = 'L';
    pub_turn_.publish(msg_cmd_turn_);
    // move backward with speed_parking_backward
    msg_cmd_move_.data = speed_parking_backward;
    pub_move_.publish(msg_cmd_move_);

    ros::Rate looprate(20);

    // check and change car posture with a rate of 20Hz until car rear is in parking space
    while (ros::ok())
    {
        // publish move and turn commands
        pub_move_.publish(msg_cmd_move_);
        pub_turn_.publish(msg_cmd_turn_);

        // car rear is already in parking space
        // (sensor upa_bcl is in the middle of the parking space)
        if (msg_upa_bcl_.range < parallel_length/2)
        {
            // turn full right
            msg_cmd_turn_.data = 'R';
            pub_turn_.publish(msg_cmd_turn_);

            break;      // break this loop
        }
        // car rear is reaching parking space
        // (the actuall angle between car and parking space depends on the starting distance)
        else if (msg_upa_bcr_.range < parallel_length)
        {
            // turn straight
            msg_cmd_turn_.data = 'D';
            pub_turn_.publish(msg_cmd_turn_);
        }
        else
        {
            // do nothing
        }

        looprate.sleep();
    }

    // check and change car posture with a rate of 20Hz until parking is finished
    while (ros::ok())
    {
        // publish move and turn commands
        pub_move_.publish(msg_cmd_move_);
        pub_turn_.publish(msg_cmd_turn_);

        // car is parallel to the parkwall on the left side
        if (fabs(msg_apa_lb_.range - msg_apa_lb2_.range) <= apa_tolerance)
        {
            // turn straight
            msg_cmd_turn_.data = 'D';
            pub_turn_.publish(msg_cmd_turn_);
            // keep moving forward with speed_parking_forward
            msg_cmd_move_.data = speed_parking_forward;
            pub_move_.publish(msg_cmd_move_);

            // car is close to parkwall (car) at front
            if (min(msg_upa_fcl_.range, msg_upa_fcr_.range) < parking_distance_min)
            {
                // stop
                msg_cmd_move_.data = 0;
                pub_move_.publish(msg_cmd_move_);

                parking_finished = true;     // parking finished!
                break;      // break this loop
            }
        }
        // car is not parallel to the parkwall on the left side
        else
        {
            // car moves forard
            if (msg_car_speed_.data > 0)
            {
                // car head is closer to the left parkwall (car) than car rear
                if ((msg_apa_lb_.range - msg_apa_lb2_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'R')
                    {
                        // turn a little right
                        msg_cmd_turn_.data = 'r';
                    }
                }
                // car rear is closer to the left parkwall (car) than car head
                else if ((msg_apa_lb2_.range - msg_apa_lb_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'L')
                    {
                        // turn a little left
                        msg_cmd_turn_.data = 'l';
                    }
                }
                else
                {
                    // do nothing
                }

                // car head is too close to parkwall (car)
                if (min(msg_apa_lf_.range, msg_upa_fl_.range) < parking_distance_min)
                {
                    // stop
                    msg_cmd_move_.data = 0;
                    pub_move_.publish(msg_cmd_move_);
                    // turn full right
                    msg_cmd_turn_.data = 'R';
                    pub_turn_.publish(msg_cmd_turn_);
                    // move backward with speed_parking_backward
                    msg_cmd_move_.data = speed_parking_backward;
                    pub_move_.publish(msg_cmd_move_);
                }
            }
            // car moves backward
            else if (msg_car_speed_.data < 0)
            {
                // car head is closer to the left parkwall (car) than car rear
                if ((msg_apa_lb_.range - msg_apa_lb2_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'L')
                    {
                        // turn a little left
                        msg_cmd_turn_.data = 'l';
                    }
                }
                // car rear is closer to the left parkwall (car) than car head
                else if ((msg_apa_lb2_.range - msg_apa_lb_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'R')
                    {
                        // turn a little right
                        msg_cmd_turn_.data = 'r';
                    }
                }
                else
                {
                    // do nothing
                }

                // car head is too close to parkwall (car)
                if (min(msg_apa_lf_.range, msg_upa_fl_.range) < parking_distance_min)
                {
                    // turn straight
                    msg_cmd_turn_.data = 'D';
                    pub_turn_.publish(msg_cmd_turn_);
                }
                else
                {
                    // turn full right
                    msg_cmd_turn_.data = 'R';
                    pub_turn_.publish(msg_cmd_turn_);
                }

                // car rear is too close to parkwall (car)
                if (min(msg_apa_lb_.range, msg_upa_bl_.range) < parking_distance_min)
                {
                    // stop
                    msg_cmd_move_.data = 0;
                    pub_move_.publish(msg_cmd_move_);
                    // turn full left
                    msg_cmd_turn_.data = 'L';
                    pub_turn_.publish(msg_cmd_turn_);
                    // move forward
                    msg_cmd_move_.data = speed_parking_forward;
                    pub_move_.publish(msg_cmd_move_);
                }
            }
            // car stops
            else
            {
                // do nothing
            }
        }

        looprate.sleep();
    }
}


// *****************************************************
// function of parallel parking on the right side
// *****************************************************
void ParkingIn::parallel_parking_right()
{
    // stop
    msg_cmd_move_.data = 0;
    pub_move_.publish(msg_cmd_move_);
    // turn full right
    msg_cmd_turn_.data = 'R';
    pub_turn_.publish(msg_cmd_turn_);
    // move backward with speed_parking_backward
    msg_cmd_move_.data = speed_parking_backward;
    pub_move_.publish(msg_cmd_move_);

    ros::Rate looprate(20);

    // check and change car posture with a rate of 20Hz until car rear is in parking space
    while (ros::ok())
    {
        // publish move and turn commands
        pub_move_.publish(msg_cmd_move_);
        pub_turn_.publish(msg_cmd_turn_);

        // car rear is already in parking space
        // (sensor upa_bcr is in the middle of the parking space)
        if (msg_upa_bcr_.range < parallel_length/2)
        {
            // turn full left
            msg_cmd_turn_.data = 'L';
            pub_turn_.publish(msg_cmd_turn_);

            break;      // break this loop
        }
        // car rear is reaching parking space
        // (the actuall angle between car and parking space depends on the starting distance)
        else if (msg_upa_bcr_.range < parallel_length)
        {
            // turn straight
            msg_cmd_turn_.data = 'D';
            pub_turn_.publish(msg_cmd_turn_);
        }
        else
        {
            // do nothing
        }

        looprate.sleep();
    }

    // check and change car posture with a rate of 20Hz until parking is finished
    while (ros::ok())
    {
        // publish move and turn commands
        pub_move_.publish(msg_cmd_move_);
        pub_turn_.publish(msg_cmd_turn_);

        // car is parallel to the parkwall on the right side
        if (fabs(msg_apa_rb_.range - msg_apa_rb2_.range) <= apa_tolerance)
        {
            // turn straight
            msg_cmd_turn_.data = 'D';
            pub_turn_.publish(msg_cmd_turn_);
            // keep moving forward with speed_parking_forward
            msg_cmd_move_.data = speed_parking_forward;
            pub_move_.publish(msg_cmd_move_);

            // car is close to parkwall (car) at front
            if (min(msg_upa_fcl_.range, msg_upa_fcr_.range) < parking_distance_min)
            {
                // stop
                msg_cmd_move_.data = 0;
                pub_move_.publish(msg_cmd_move_);

                parking_finished = true;     // parking finished!
                break;      // break this loop
            }
        }
        // car is not parallel to the parkwall on the right side
        else
        {
            // car moves forard
            if (msg_car_speed_.data > 0)
            {
                // car head is closer to the right parkwall (car) than car rear
                if ((msg_apa_rb_.range - msg_apa_rb2_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'L')
                    {
                        // turn a little left
                        msg_cmd_turn_.data = 'l';
                    }
                }
                // car rear is closer to the right parkwall (car) than car head
                else if ((msg_apa_rb2_.range - msg_apa_rb_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'R')
                    {
                        // turn a little right
                        msg_cmd_turn_.data = 'r';
                    }
                }
                else
                {
                    // do nothing
                }

                // car head is too close to parkwall (car)
                if (min(msg_apa_rf_.range, msg_upa_fr_.range) < parking_distance_min)
                {
                    // stop
                    msg_cmd_move_.data = 0;
                    pub_move_.publish(msg_cmd_move_);
                    // turn full left
                    msg_cmd_turn_.data = 'L';
                    pub_turn_.publish(msg_cmd_turn_);
                    // move backward with speed_parking_backward
                    msg_cmd_move_.data = speed_parking_backward;
                    pub_move_.publish(msg_cmd_move_);
                }
            }
            // car moves backward
            else if (msg_car_speed_.data < 0)
            {
                // car head is closer to the right parkwall (car) than car rear
                if ((msg_apa_rb_.range - msg_apa_rb2_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'R')
                    {
                        // turn a little right
                        msg_cmd_turn_.data = 'r';
                    }
                }
                // car rear is closer to the right parkwall (car) than car head
                else if ((msg_apa_rb2_.range - msg_apa_rb_.range) > apa_tolerance)
                {
                    if (msg_cmd_turn_.data != 'L')
                    {
                        // turn a little left
                        msg_cmd_turn_.data = 'l';
                    }
                }
                else
                {
                    // do nothing
                }

                // car head is too close to parkwall (car)
                if (min(msg_apa_rf_.range, msg_upa_fr_.range) < parking_distance_min)
                {
                    // turn straight
                    msg_cmd_turn_.data = 'D';
                    pub_turn_.publish(msg_cmd_turn_);
                }
                else
                {
                    // turn full left
                    msg_cmd_turn_.data = 'L';
                    pub_turn_.publish(msg_cmd_turn_);
                }

                // car rear is too close to parkwall (car)
                if (min(msg_apa_rb_.range, msg_upa_br_.range) < parking_distance_min)
                {
                    // stop
                    msg_cmd_move_.data = 0;
                    pub_move_.publish(msg_cmd_move_);
                    // turn full right
                    msg_cmd_turn_.data = 'R';
                    pub_turn_.publish(msg_cmd_turn_);
                    // move forward
                    msg_cmd_move_.data = speed_parking_forward;
                    pub_move_.publish(msg_cmd_move_);
                }
            }
            // car stops
            else
            {
                // do nothing
            }
        }

        looprate.sleep();
    }
}


int main(int argc, char **argv)
{
    ros::init(argc, argv, "parking_in");

    // create a node handle, use global callback queue
    ros::NodeHandle nh;
    //create a node handle for class, use custom callback queue
    ros::NodeHandle nh_c;
    // create custom callback queue
    ros::CallbackQueue callback_queue;
    // set custom callback queue
    nh_c.setCallbackQueue(&callback_queue);

    // create a subscriber for topic "parking_space"
    ros::Subscriber sub_parking_space = nh.subscribe<std_msgs::Header>("parking_space", 1, \
    callback_parking_space);

    // instantiating class object
    ParkingIn ParkingIn_obj(&nh_c);  // pass nh_c to class constructor

    // create AsyncSpinner, run it on all available cores to process custom callback queue
    sp_spinner.reset(new ros::AsyncSpinner(0, &callback_queue));

    // set loop rate
    ros::Rate loop_rate(10);
    while (ros::ok())
    {
        if (choosed_parking_space != 0)
        {
            // save the choosed parking space for parking out
            parking_space = choosed_parking_space;

            if (!trigger_spinner)
            {
                // clear old callbacks in custom callback queue
                callback_queue.clear();
                // start spinners for custom callback queue
                sp_spinner->start();
                ROS_INFO("Spinners enabled in parking_in");

                trigger_spinner = true;
            }
            else
            {
                if (parking_finished)
                {
                    // stop spinners for custom callback queue
                    sp_spinner->stop();
                    ROS_INFO("Spinners disabled in parking_in");

                    // reset
                    trigger_spinner = false;
                    choosed_parking_space = 0;
                    parking_finished = false;
                    time_out = false;

                    // could shutdown this node after parking
                    //ros::shutdown();
                }
                else
                {
                    if (time_out)
                    {
                        ROS_INFO("time of %f[s] for parking is over", parking_time);
                        // here can ask the driver if quit parking:
                        // do move back to the street and go on searching parking space
                        // stop spinners for custom callback queue
                        // sp_spinner->stop();

                        // or continue parking:
                        // do continue
                    }
                }
            }

            // check parking_space and park in
            switch (choosed_parking_space)
            {
            case SPACE_LEFT_PERPENDICULAR:
                ROS_INFO("perpendicular_parking_left start");
                // move forward before perpendicular parking in
                ParkingIn_obj.move_before_parking(move_distance_perpendicular);
                // perpendicular parking on the left side
                ParkingIn_obj.perpendicular_parking_left();
                break;

            case SPACE_RIGHT_PERPENDICULAR:
                ROS_INFO("perpendicular_parking_right start");
                // move forward before perpendicular parking in
                ParkingIn_obj.move_before_parking(move_distance_perpendicular);
                // perpendicular parking on the right side
                ParkingIn_obj.perpendicular_parking_right();
                break;

            case SPACE_LEFT_PARALLEL:
                ROS_INFO("parallel_parking_left start");
                // parallel parking on the left side
                ParkingIn_obj.parallel_parking_left();
                break;

            case SPACE_RIGHT_PARALLEL:
                ROS_INFO("parallel_parking_right start");
                // parallel parking on the right side
                ParkingIn_obj.parallel_parking_right();
                break;

            default:
                break;
            }
        }

        // process message in global callback queue: "parking_enable"
        ros::spinOnce();

        loop_rate.sleep();
    }

    // release AsyncSpinner object
    sp_spinner.reset();

    // wait for ROS threads to terminate
    ros::waitForShutdown();

    return 0;
}