#define thresh4black 100
#define distance4objects 10
#define gap 5
#define div4k 0.09
#define sum4k 11
#define zero4k 0.5
#define cut_height 10
#define cut_width 10
#define get1 0
#define get2 220
#include<opencv.hpp>
#include<opencv2/core/core.hpp>
#include<iostream>
#include"SerialPort.h"
using namespace cv;
using namespace std;

//frame.cols//640//resize//320
//frame.rows//480//resize//240

bool sendCoorFire(int Fire, CSerialPort&CS);

int main()
{
	//串口通信初始化
	CSerialPort mySerialPort;
	if (!mySerialPort.InitPort(5, CBR_115200))
	{
		std::cout << "initPort fail !" << std::endl;
	}
	else
	{
		std::cout << "initPort success !" << std::endl;
	}
	if (!mySerialPort.OpenListenThread())
	{
		std::cout << "OpenListenThread fail !" << std::endl;
	}
	else
	{
		std::cout << "OpenListenThread success !" << std::endl;
	}

	VideoCapture capture(1);
	Mat frame;
	double FPS = 0, t = 0;
	vector<Point2i> objects_top_point;
	vector<Point2i> first_obj;
	vector<Point2i> second_obj;
	vector<Point2i> third_obj;

	while (1)
	{
		t = (double)getTickCount();
		capture >> frame;
		Mat gray_frame(frame.size(), frame.type());
		Mat show_frame = frame;
		//Mat frame_hsv(frame.size(), frame.type());
		//cvtColor(frame, frame_hsv, CV_BGR2HSV_FULL);
		cvtColor(frame, gray_frame, CV_BGR2GRAY);
		//imwrite("hsv.jpg", frame_hsv);
		resize(gray_frame, gray_frame, Size(0, 0), 0.5, 0.5);
		resize(show_frame, show_frame, Size(0, 0), 0.5, 0.5);

		line(show_frame, Point(get1, 0), Point(get1, show_frame.rows-1), Scalar(155, 255, 255), 3, 8, 0);
		line(show_frame, Point(get2, 0), Point(get2, show_frame.rows-1), Scalar(155, 255, 255), 3, 8, 0);
		for (int i = 0;i < gray_frame.cols;i++)//砍掉上部分
		{
			for (int j = 0;j < cut_height;j++)
			{
				gray_frame.at<uchar>(j, i) = 255;
			}
		}
		for (int i = 0;i < gray_frame.cols;i++)//砍掉下部分
		{
			for (int j = gray_frame.rows - 1;j > gray_frame.rows - cut_height;j--)
			{
				gray_frame.at<uchar>(j, i) = 255;
			}
		}
		for (int i = 0;i < cut_width;i++)//砍掉左部分
		{
			for (int j = 0;j < gray_frame.rows;j++)
			{
				gray_frame.at<uchar>(j, i) = 255;
			}
		}
		for (int i = gray_frame.cols - 1;i > gray_frame.cols - cut_width;i--)//砍掉右部分
		{
			for (int j = 0;j < gray_frame.rows;j++)
			{
				gray_frame.at<uchar>(j, i) = 255;
			}
		}
		imshow("aa", gray_frame);

		for (int i = 0;i < gray_frame.cols;i++)
		{
		    for (int j = 0;j < gray_frame.rows;j++)
			{
				int temp_point = gray_frame.at<uchar>(j, i);
				if (temp_point < thresh4black)
				{
					gray_frame.at<uchar>(j, i) = 255;
				}
				else
				gray_frame.at<uchar>(j, i) = 0;
			}
		}
		medianBlur(gray_frame, gray_frame, 3);
		//找上边界点
		for (int i = 0;i < gray_frame.cols;i++)
		{
			for (int j = 0;j < gray_frame.rows;j++)
			{
				if (gray_frame.at<uchar>(j, i) == 255)
				{
					objects_top_point.push_back(Point(i, j));
					break;
				}
			}
		}
		if (objects_top_point.size() > 1)
		{
			int top_point_first_x = objects_top_point[0].x;
			int top_point_first_y = objects_top_point[0].y;
			int top_point_last_x = objects_top_point[objects_top_point.size() - 1].x;
			int top_point_last_y = objects_top_point[objects_top_point.size() - 1].y;
			if (top_point_first_x != 0 && top_point_first_x != (gray_frame.cols - 1) && top_point_last_x != 0 && top_point_last_x != (gray_frame.cols - 1))
			{
				//分割图像块
				int temp_obj = 1;
				for (int i = 0;i < objects_top_point.size() - 1;i++)
				{
					if (objects_top_point[i + 1].x - objects_top_point[i].x < distance4objects)
					{
						if (temp_obj == 1)
						{
							first_obj.push_back(objects_top_point[i]);
						}
						if (temp_obj == 2)
						{
							second_obj.push_back(objects_top_point[i]);
						}
						if (temp_obj == 3)
						{
							third_obj.push_back(objects_top_point[i]);
						}
					}
					else
					{
						++temp_obj;
					}
				}
				//cout << "first " << first_obj.size() << endl;
				//cout << "second " << second_obj.size() << endl;
				//cout << "third " << third_obj.size() << endl;

				//判断各个图像块的斜率
				int temp_k = 0;
				float x1 = 0, x2 = 0, y1 = 0, y2 = 0, k = 0, k_sum = 0, k_time = 0, k_div = 0, k_zero = 0;
				if (first_obj.size() > 20 && temp_k == 0)
				{
					for (int i = 0;i < first_obj.size() - 1;i++)
					{
						x1 = first_obj[i].x;
						y1 = first_obj[i].y;
						x2 = first_obj[i + 1].x;
						y2 = first_obj[i + 1].y;
						k = (y2 - y1) / (x2 - x1);
						//cout << k << endl;
						if (abs(k) < 3)
						{
							k_sum = k + k_sum;
							++k_time;
						}
						if (k == 0)
						{
							++k_zero;
						}
					}
					k_div = k_sum / k_time;
					k_zero = k_zero / k_time;
					cout << "k_div " << abs(k_div) << endl;
					cout << "k_sum " << abs(k_sum) << endl;
					cout << "k_zero " << k_zero << endl;
					if (abs(k_div) < div4k && abs(k_sum) < sum4k && k_zero < zero4k)
					{
						temp_k = 1;
						Point2i P;
						int temp_i = (int)(first_obj.size() / 2);
						P = first_obj[temp_i];
						int R = (int)((first_obj[first_obj.size() - 1].x - first_obj[0].x) / 2);
						P.y = P.y + R;
						cout << "print " << R << endl;
						circle(show_frame, P, R, Scalar(155, 255, 255), 3, 8);
						if(P.x>=get1&&P.x<=get2) sendCoorFire(R, mySerialPort);	
					}
					else
					{
						k_time = 0;
						k_div = 0;
						k_sum = 0;
						k_zero = 0;
					}
				}
				if (second_obj.size() > 20 && temp_k == 0)
				{
					for (int i = 0;i < second_obj.size() - 1;i++)
					{
						x1 = second_obj[i].x;
						y1 = second_obj[i].y;
						x2 = second_obj[i + 1].x;
						y2 = second_obj[i + 1].y;
						k = (y2 - y1) / (x2 - x1);
						//cout << k << endl;
						if (abs(k) < 3)
						{
							k_sum = k + k_sum;
							++k_time;
						}
						if (k == 0)
						{
							++k_zero;
						}
					}
					k_div = k_sum / k_time;
					k_zero = k_zero / k_time;
					if (abs(k_div) < div4k && abs(k_sum) < sum4k && k_zero < zero4k)
					{
						temp_k = 1;
						Point2i P;
						int temp_i = (int)(second_obj.size() / 2);
						P = second_obj[temp_i];
						int R = (int)((second_obj[second_obj.size() - 1].x - second_obj[0].x) / 2);
						P.y = P.y + R;
						cout << "print " << R << endl;
						circle(show_frame, P, R, Scalar(155, 255, 255), 3, 8);
						if (P.x >= get1&&P.x <= get2) sendCoorFire(R, mySerialPort);
					}
					else
					{
						k_time = 0;
						k_div = 0;
						k_sum = 0;
						k_zero = 0;
					}
				}
				if (third_obj.size() > 20 && temp_k == 0)
				{
					for (int i = 0;i < third_obj.size() - 1;i++)
					{
						x1 = third_obj[i].x;
						y1 = third_obj[i].y;
						x2 = third_obj[i + 1].x;
						y2 = third_obj[i + 1].y;
						k = (y2 - y1) / (x2 - x1);
						//cout << k << endl;
						if (abs(k) < 3)
						{
							k_sum = k + k_sum;
							++k_time;
						}
						if (k == 0)
						{
							++k_zero;
						}
					}
					k_div = k_sum / k_time;
					k_zero = k_zero / k_time;
					if (abs(k_div) < div4k && abs(k_sum) < sum4k && k_zero < zero4k)
					{
						temp_k = 1;
						Point2i P;
						int temp_i = (int)(third_obj.size() / 2);
						P = third_obj[temp_i];
						int R = (int)((third_obj[third_obj.size() - 1].x - third_obj[0].x) / 2);
						P.y = P.y + R;
						cout << "print " << R << endl;
						circle(show_frame, P, R, Scalar(155, 255, 255), 3, 8);
						if (P.x >= get1&&P.x <= get2) sendCoorFire(R, mySerialPort);
					}
					else
					{
						k_time = 0;
						k_div = 0;
						k_sum = 0;
						k_zero = 0;
					}
				}
				sendCoorFire(0, mySerialPort);
			}
		}

		//清空
		objects_top_point.clear();
		first_obj.clear();
		second_obj.clear();
		third_obj.clear();
		//计算帧率
		t = ((double)getTickCount() - t) / getTickFrequency();
		FPS = 1.0 / t;
		std::cout << "FPS " << FPS << endl;
		cv::imshow("gray_frame", gray_frame);
		cv::imshow("读取视频", show_frame);
		cv::waitKey(30);
	}
	return 0;
}
//sendCoorFire(1, mySerialPort)
bool sendCoorFire(int Fire, CSerialPort&CS)
{
	unsigned char string[1];
	if (0 < Fire && Fire <= 12)
	{
		string[0] = '1';
		cout << string[0] << endl;
		return CS.WriteData(string, 1);
	}
	if (12 < Fire && Fire <= 16)
	{
		string[0] = '2';
		cout << string[0] << endl;
		return CS.WriteData(string, 1);
	}
	if (16 < Fire && Fire <= 22)
	{
		string[0] = '3';
		cout << string[0] << endl;
		return CS.WriteData(string, 1);
	}
	if (22 < Fire)
	{
		string[0] = '4';
		cout << string[0] << endl;
		return CS.WriteData(string, 1);
	}
	else
	{
		return 0;
	}
}