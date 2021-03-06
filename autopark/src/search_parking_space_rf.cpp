/******************************************************************
 * Filename: search_parking_space_rf.cpp
 * Version: v1.0
 * Author: Meng Peng
 * Date: 2020-04-30
 * Description: search parking space on the right side with apa_rf
 * 
 ******************************************************************/

#include "autopark/autoparking.h"
#include "autopark/search_parking_space_rf.h"

using namespace std;

static bool parking_enable = false;     // flag of parking enable
static bool search_done = false;        // flag of searching parking space done
static bool trigger_spinner = false;    // flag of spinners enable

static float distance_min;              // minimum distance between car and object

boost::shared_ptr<ros::AsyncSpinner> sp_spinner;   // create a shared_ptr for AsyncSpinner object

// define struct of Times for stop situation during check parking space
struct Times
{
    ros::Time begin;
    ros::Time end;
    double duration;
    bool trigger;
} time_2, time_5;    // define two objects for case 2 and case 5


// callback from global callback queue
// callback of sub_parking_enable
void callback_parking_enable(const std_msgs::Bool::ConstPtr& msg)
{
    ROS_INFO("call callback of parking_enable: %d", msg->data);
    parking_enable = msg->data;
}

// callback of sub_search_done
void callback_search_done(const std_msgs::Bool::ConstPtr& msg)
{
    ROS_INFO("call callback of search_done: %d", msg->data);
    search_done = msg->data;
}


// CONSTRUCTOR: called when this object is created to set up subscribers and publishers
SearchParkingSpaceRF::SearchParkingSpaceRF(ros::NodeHandle* nodehandle):nh_(*nodehandle)
{
    ROS_INFO("call constructor in search_parking_space_rf");

    sub_apa_rf_ = nh_.subscribe<sensor_msgs::Range>("apa_rf", 1, \
    &SearchParkingSpaceRF::callback_apa_rf, this);

    sub_car_speed_ = nh_.subscribe<std_msgs::Float32>("car_speed", 1, \
    &SearchParkingSpaceRF::callback_car_speed, this);

    pub_parking_space_ = nh_.advertise<std_msgs::Header>("parking_space_rf", 1);
}

// DESTRUCTOR: called when this object is deleted to release memory 
SearchParkingSpaceRF::~SearchParkingSpaceRF(void)
{
    ROS_INFO("call destructor in search_parking_space_rf");
}

// callbacks from custom callback queue
// callback of sub_car_speed_
void SearchParkingSpaceRF::callback_car_speed(const std_msgs::Float32::ConstPtr& msg)
{
    ROS_INFO("call callback of car_speed: speed=%f", msg->data);
    msg_car_speed_.data = msg->data;

    if (vec_turnpoint_.size() == 1)
    {
        if (msg_car_speed_.data == 0 && !time_2.trigger)
        {
            time_2.begin = ros::Time::now();
            time_2.trigger = true;
        }
        if (msg_car_speed_.data != 0 && time_2.trigger)
        {
            time_2.end = ros::Time::now();
            time_2.trigger = false;

            time_2.duration += (time_2.end - time_2.begin).toSec();
        }
    }
    if (vec_turnpoint_.size() == 3)
    {
        if (msg_car_speed_.data == 0 && !time_5.trigger)
        {
            time_5.begin = ros::Time::now();
            time_5.trigger = true;
        }
        if (msg_car_speed_.data != 0 && time_5.trigger)
        {
            time_5.end = ros::Time::now();
            time_5.trigger = false;

            time_5.duration += (time_5.end - time_5.begin).toSec();
        }
    }
}

// callback of sub_apa_rf_
void SearchParkingSpaceRF::callback_apa_rf(const sensor_msgs::Range::ConstPtr& msg)
{
    ROS_INFO("call callback of apa_rf: range=%f", msg->range);
    msg_apa_rf_.header.stamp = msg->header.stamp;
    msg_apa_rf_.header.frame_id = msg->header.frame_id;
    msg_apa_rf_.range = msg->range;

    // start check function
    check_parking_space();
}

// check function to find parking space
void SearchParkingSpaceRF::check_parking_space()
{
    ROS_INFO("call check function in search_parking_space_rf");
    if (que_apa_rf_.empty())
    {
        //detected object in the range of distance_search
        if (msg_apa_rf_.range < distance_search)
        {
            // add first valid range message to que_apa_rf_
            que_apa_rf_.push(msg_apa_rf_);
        }
    }
    else
    {
        // add new message to que_apa_rf_
        que_apa_rf_.push(msg_apa_rf_);

        // number of turn points in vec_turnpoint_
        static uint16_t size_vec_turnpoint = vec_turnpoint_.size();
        switch (size_vec_turnpoint)
        {
        case 0:     // no turn point in vec_turnpoint_
            // range increase: close to parking space (parking gap)
            if ((que_apa_rf_.back().range - que_apa_rf_.front().range) > range_diff)
            {
                // do nothing
            }
            // range decrease: away from parking space (parking gap)
            else if ((que_apa_rf_.front().range - que_apa_rf_.back().range) > range_diff)
            {
                vec_turnpoint_.push_back(que_apa_rf_.back());  // first turn point
                distance_min = que_apa_rf_.back().range;
            }
            // range is stable
            else
            {
                vec_turnpoint_.push_back(que_apa_rf_.front());   // first turn point
                distance_min = que_apa_rf_.front().range;
            }
            break;

        case 1:     // one turn point in vec_turnpoint_
            // range increase: close to parking space (parking gap)
            if ((que_apa_rf_.back().range - que_apa_rf_.front().range) > range_diff)
            {
                vec_turnpoint_.push_back(que_apa_rf_.front());    // second turn point
            }
            // range decrease: away from parking space (parking gap)
            else if ((que_apa_rf_.front().range - que_apa_rf_.back().range) > range_diff)
            {
                vec_turnpoint_.pop_back();      // delete the added first turn point
                vec_turnpoint_.push_back(que_apa_rf_.back());  // add the new first turn point
                distance_min = que_apa_rf_.back().range;
            }
            else
            {
                if (que_apa_rf_.back().range < distance_min)
                {
                    distance_min = que_apa_rf_.back().range;  // get new distance_min
                }
            }
            break;

        case 2:     // two turn points in vec_turnpoint_
            double obj_diff, obj_length;
            // time duration between the first and second turn point
            obj_diff = (vec_turnpoint_[1].header.stamp - vec_turnpoint_[0].header.stamp).toSec();
            // parallel length of object
            obj_length = msg_car_speed_.data * (obj_diff - time_2.duration);
            // reset time duration for stop
            time_2.duration = 0;
            
            if (obj_length < perpendicular_width && obj_length > car_width)
            {
                // perpendicular parking: 0 0 0 0  0 0 0 1
                setbit(msg_parking_space_.seq, 0);
                clrbit(msg_parking_space_.seq, 1);
            }
            else if (obj_length < parallel_width && obj_length > car_length)
            {
                // parallel parking: 0 0 0 0  0 0 1 0
                setbit(msg_parking_space_.seq, 1);
                clrbit(msg_parking_space_.seq, 0);
            }
            else
            {
                // do nothing
            }

            // range increase: close to parking space (parking gap)
            if ((que_apa_rf_.back().range - que_apa_rf_.front().range) > range_diff)
            {
                // do nothing
            }
            // range decrease: away from parking space (parking gap)
            else if ((que_apa_rf_.front().range - que_apa_rf_.back().range) > range_diff)
            {
                vec_turnpoint_.push_back(que_apa_rf_.front());    // third key point
                vec_turnpoint_.push_back(que_apa_rf_.front());    // fourth key point
            }
            // range is stable
            else
            {
                vec_turnpoint_.push_back(que_apa_rf_.front());   // third turn point
            }
            break;

        case 3:     // three turn points in vec_turnpoint_
            // range decrease: away from parking space (parking gap)
            if ((que_apa_rf_.front().range - que_apa_rf_.back().range) > range_diff)
            {
                vec_turnpoint_.push_back(que_apa_rf_.front());     // fourth turn point
            }
            /* the only way to find the fourth turn point */
            else
            {
                // do nothing
            }
            break;

        case 4:     // four turn points in vec_turnpoint_
            // range increase: close to parking space (parking gap)
            if ((que_apa_rf_.back().range - que_apa_rf_.front().range) > range_diff)
            {
                // do nothing
            }
            // range decrease: away from parking space (parking gap)
            else if ((que_apa_rf_.front().range - que_apa_rf_.back().range) > range_diff)
            {
                // do nothing
            }
            // range is stable
            else
            {
                vec_turnpoint_.push_back(que_apa_rf_.front());     // fifth turn point
            }
            /* the only way to find the fifth turn point */
            break;

        case 5:     // five turn points in vec_turnpoint_
            double space_diff, space_width, space_length;
            // time duration between the third and fourth turn point
            space_diff = (vec_turnpoint_[4].header.stamp - vec_turnpoint_[3].header.stamp).toSec();
            // width of space
            space_width = msg_car_speed_.data * (space_diff - time_5.duration);
            // reset time duration for stop
            time_5.duration = 0;

            // length of space
            space_length = min(vec_turnpoint_[3].range, vec_turnpoint_[4].range) - \
            min(distance_min, vec_turnpoint_[5].range);

            if ((space_width > perpendicular_width && space_length > perpendicular_length) || \
            (space_width > parallel_width && space_length > parallel_length))
            {
                // parking space on the right side
                setbit(msg_parking_space_.seq, 2);     // 0 0 0 0  0 1 x x
                // 0 1 0 1  0 0 0 0: perpendicular parking on the right side
                // 0 1 1 0  0 0 0 0: parallel parking space on the right side
                // else: not a valid parking space

                // in case of specific parking space with 3 walls
                // perpendicular parking space
                if (space_width < parallel_width)
                {
                    // perpendicular parking: 0 0 0 0  0 0 0 1
                    setbit(msg_parking_space_.seq, 0);
                    clrbit(msg_parking_space_.seq, 1);
                }
                // parallel parking space
                if (space_length < perpendicular_length)
                {
                    // parallel parking: 0 0 0 0  0 0 1 0
                    setbit(msg_parking_space_.seq, 1);
                    clrbit(msg_parking_space_.seq, 0);
                }

                // set time stamp
                msg_parking_space_.stamp = ros::Time::now();
                // publish parking space
                pub_parking_space_.publish(msg_parking_space_);
                // reset parking space state
                clrbit(msg_parking_space_.seq, 2);     // 0 0 0 0  0 0 x x
            }

            // delete fist message in que_apa_rf_
            que_apa_rf_.pop();
            // clear all messages in vec_turnpoint_
            vec_turnpoint_.clear();
            break;

        default:
            break;
        }

        if(!(que_apa_rf_.empty()))
            que_apa_rf_.pop();     // delete first message in to keep queue_size less than 2
    }
}


int main(int argc, char **argv)
{
    ros::init(argc, argv, "search_parking_space_rf");

    // create a node handle, use global callback queue
    ros::NodeHandle nh;
    //create a node handle for class, use custom callback queue
    ros::NodeHandle nh_c;
    // create custom callback queue
    ros::CallbackQueue callback_queue;
    // set custom callback queue
    nh_c.setCallbackQueue(&callback_queue);

    ros::Subscriber sub_parking_enable = nh.subscribe<std_msgs::Bool>("parking_enable", 1, \
    callback_parking_enable);

    ros::Subscriber sub_search_done = nh.subscribe<std_msgs::Bool>("search_done", 1, \
    callback_search_done);

    // instantiating an object of class SearchParkingSpaceRF
    SearchParkingSpaceRF SearchParkingSpaceRF_rf(&nh_c);  // pass nh_c to class constructor

    // create AsyncSpinner, run it on all available cores to process custom callback queue
    sp_spinner.reset(new ros::AsyncSpinner(0, &callback_queue));

    // initialize Times: 
    time_2.duration = 0;
    time_2.trigger = false;

    time_5.duration = 0;
    time_5.trigger = false;

    // set loop rate
    ros::Rate loop_rate(10);
    while (ros::ok())
    {
        ROS_INFO_STREAM_ONCE("node search_parking_space_rf is running");
        if (parking_enable)
        {
            if (!trigger_spinner)
            {
                ROS_INFO("search parking space with apa_rf enabled");
                // clear old callbacks in custom callback queue
                callback_queue.clear();
                // start spinners for custom callback queue
                sp_spinner->start();
                ROS_INFO("spinners start");

                trigger_spinner = true;
            }
            else
            {
                if (search_done)
                {
                    ROS_INFO("search parking space finished");
                    // stop spinners for custom callback queue
                    sp_spinner->stop();
                    ROS_INFO("spinners stop");

                    // reset
                    parking_enable = false;
                    search_done = false;
                    trigger_spinner = false;
                }
            }
        }
        else
        {
            if (trigger_spinner)
            {
                ROS_INFO_STREAM_ONCE("search parking space with apa_rf disabled");
                // stop spinners for custom callback queue
                sp_spinner->stop();
                ROS_INFO("spinners stop");

                // reset
                parking_enable = false;
                search_done = false;
                trigger_spinner = false;
            }
        }

        ros::spinOnce();

        loop_rate.sleep();
    }

    // release AsyncSpinner object
    sp_spinner.reset();

    // wait for ROS threads to terminate
    ros::waitForShutdown();

    return 0;
}