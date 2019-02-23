#pragma once
#include"opencv.hpp"
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <cstring>
#include<fstream>
#include <opencv2/tracking/tracker.hpp>
namespace WinForm_TrackTest {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace cv;
	using namespace std;
	VideoCapture cap;
	MultiTracker tracker;

	vector<Rect2d> objects;
	fstream fp;
	bool Is_initial = false;
	/// <summary>
	/// MyForm 的摘要
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			//
			//TODO:  在此加入建構函式程式碼
			//
			cap.open("GICT1492.avi");
			fp.open("objectPos3.txt", ios::in);
		}

	protected:
		/// <summary>
		/// 清除任何使用中的資源。
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::PictureBox^  pictureBox1;
	protected:
	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::Timer^  timer1;





	private: System::ComponentModel::IContainer^  components;

	private:
		/// <summary>
		/// 設計工具所需的變數。
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// 此為設計工具支援所需的方法 - 請勿使用程式碼編輯器修改
		/// 這個方法的內容。
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->BeginInit();
			this->SuspendLayout();
			// 
			// pictureBox1
			// 
			this->pictureBox1->Location = System::Drawing::Point(13, 13);
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(601, 494);
			this->pictureBox1->TabIndex = 0;
			this->pictureBox1->TabStop = false;
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(845, 92);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(75, 23);
			this->button1->TabIndex = 1;
			this->button1->Text = L"button1";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &MyForm::button1_Click);
			this->button1->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &MyForm::button1_KeyDown);
			// 
			// timer1
			// 
			this->timer1->Interval = 1;
			this->timer1->Tick += gcnew System::EventHandler(this, &MyForm::timer1_Tick);
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1038, 652);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->pictureBox1);
			this->Name = L"MyForm";
			this->Text = L"MyForm";
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &MyForm::MyForm_KeyDown);
			this->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &MyForm::MyForm_KeyPress);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion
		bool predicate(Rect2d R1, Rect2d R2) //重疊率有沒有大於50個piexl
		{
			return   ((R1 & R2).area() / R1.area()) > 0.5;
		}

		int EuclidCluster(const vector<Rect2d>& _vec, vector<int>& labels)
		{
			int i, j, N = (int)_vec.size();
			const Rect2d* vec = &_vec[0];

			const int PARENT = 0;
			const int RANK = 1;

			vector<int> _nodes(N * 2);
			int(*nodes)[2] = (int(*)[2])&_nodes[0];

			// The first O(N) pass: create N single-vertex trees
			for (i = 0; i < N; i++)
			{
				nodes[i][PARENT] = -1;
				nodes[i][RANK] = 0;
			}

			for (i = 0; i < N; i++)
			{
				int root = i;

				// find root
				while (nodes[root][PARENT] >= 0)
					root = nodes[root][PARENT];

				for (j = 0; j < N; j++)
				{
					if (i == j || !predicate(vec[i], vec[j]))
						continue;
					int root2 = j;

					while (nodes[root2][PARENT] >= 0)
						root2 = nodes[root2][PARENT];

					if (root2 != root)
					{
						// unite both trees
						int rank = nodes[root][RANK], rank2 = nodes[root2][RANK];
						if (rank > rank2)
							nodes[root2][PARENT] = root;
						else
						{
							nodes[root][PARENT] = root2;
							nodes[root2][RANK] += rank == rank2;
							root = root2;
						}
						assert(nodes[root][PARENT] < 0);

						int k = j, parent;

						// compress the path from node2 to root
						while ((parent = nodes[k][PARENT]) >= 0)
						{
							nodes[k][PARENT] = root;
							k = parent;
						}

						// compress the path from node to root
						k = i;
						while ((parent = nodes[k][PARENT]) >= 0)
						{
							nodes[k][PARENT] = root;
							k = parent;
						}
					}
				}
			}

			// Final O(N) pass: enumerate classes
			labels.resize(N);
			int nclasses = 0;

			for (i = 0; i < N; i++)
			{
				int root = i;
				while (nodes[root][PARENT] >= 0)
					root = nodes[root][PARENT];
				if (nodes[root][RANK] >= 0)
					nodes[root][RANK] = ~nclasses++;
				labels[i] = ~nodes[root][RANK];
			}
			return nclasses;


		}

	private: vector<Rect2d> VirtualOD(Mat frame)
	{
		vector<Rect2d> OD;
		char line[100];
		fp.getline(line, sizeof(line), '\n');
		System::String^ str = gcnew System::String(line);
		if (str != "")
		{
			cli::array<System::String^>^result;
			cli::array<Char>^sep = gcnew cli::array<Char>{ ' ' };
			result = str->Split(sep, StringSplitOptions::RemoveEmptyEntries);
			for (uint16_t i = 0; i < result->Length; i++)
			{
				if ((i + 1) % 5 == 0)
				{

					uint ObjectType = Convert::ToInt64(result[i]);
					Point2d p1 = Point2d(Convert::ToDouble(result[i - 4]), Convert::ToDouble(result[i - 3]));
					Point2d p2 = Point2d(Convert::ToDouble(result[i - 2]), Convert::ToDouble(result[i - 1]));
					OD.push_back(Rect2d(p1, p2));
				}
			}
		}
		return OD;
	}
	public:void ShowImage(System::Windows::Forms::PictureBox^ PBox, cv::Mat Image)
	{


		Mat image_Temp;
		switch (Image.type())
		{
		case CV_8UC3:
			Image.copyTo(image_Temp);

			break;
		case CV_8UC1:
			cvtColor(Image, image_Temp, CV_GRAY2RGB);
			break;
		default:
			break;
		}
		Bitmap ^ bmpimg = gcnew Bitmap(image_Temp.cols, image_Temp.rows, System::Drawing::Imaging::PixelFormat::Format24bppRgb);
		System::Drawing::Imaging::BitmapData^ data = bmpimg->LockBits(System::Drawing::Rectangle(0, 0, image_Temp.cols, image_Temp.rows), System::Drawing::Imaging::ImageLockMode::WriteOnly, System::Drawing::Imaging::PixelFormat::Format24bppRgb);
		Byte* dstData = reinterpret_cast<Byte*>(data->Scan0.ToPointer());

		unsigned char* srcData = image_Temp.data;

		for (int row = 0; row < data->Height; ++row)
		{
			memcpy(reinterpret_cast<void*>(&dstData[row*data->Stride]), reinterpret_cast<void*>(&srcData[row*image_Temp.step]), image_Temp.cols*image_Temp.channels());
		}

		bmpimg->UnlockBits(data);
		PBox->Image = bmpimg;
		PBox->SizeMode = PictureBoxSizeMode::AutoSize;
		PBox->Refresh();
		GC::Collect();
	}
	private:vector<Rect2d> CombineTROD(vector<Rect2d> TR, vector<Rect2d>OD, Mat frame)
	{

		for (uint i = 0; i < TR.size(); i++)
			rectangle(frame, TR[i], Scalar(255, 0, 0), 2, 1);
		for (uint i = 0; i < OD.size(); i++)
			rectangle(frame, OD[i], Scalar(0, 0, 255), 2, 1);
		vector<Rect2d> combine, newCombine;
		combine = OD;
		for (uint i = 0; i < TR.size(); i++)combine.push_back(TR[i]);
		vector<int> labels;
		int nclass = EuclidCluster(combine, labels);
		for (uint i = 0; i < nclass; i++)
		{
			Rect2d tmp(0, 0, 1920, 1080);
			for (uint j = 0; j < labels.size(); j++)
			{

				if (labels[j] == i)
				{
					tmp &= combine[j];
				}
			}
			newCombine.push_back(tmp);
		}

		for (uint i = 0; i < newCombine.size(); i++)
		{
			rectangle(frame, newCombine[i], Scalar(0, 255, 0), 2, 1);
		}
		return newCombine;

	}
	private: System::Void timer1_Tick(System::Object^  sender, System::EventArgs^  e) {
		Mat frame;
		cap >> frame;
		std::vector<Ptr<Tracker> > algorithms;
		vector<Rect2d>NewOD = VirtualOD(frame);
		vector<Rect2d>NewObj;
		if (!Is_initial)
		{
			algorithms.resize(0);
			for (uint i = 0; i < NewOD.size(); i++)
				algorithms.push_back(TrackerKCF::create());

			NewObj = NewOD;

			Is_initial = true;
		}
		else
		{
			algorithms.resize(0);
			tracker.update(frame);
			vector<Rect2d> trckResult = tracker.getObjects();
			Mat frameDebug;
			frame.copyTo(frameDebug);
			NewObj = CombineTROD(trckResult, NewOD, frameDebug);
			//NewObj = CombineTROD(trckResult, NewOD, frame);
			for (uint i = 0; i < NewOD.size(); i++)
				algorithms.push_back(TrackerKCF::create());
		}
		tracker.clear();
	
		
		tracker.add(algorithms, frame, NewObj);





		for (unsigned i = 0; i < NewObj.size(); i++)
			rectangle(frame, NewObj[i], Scalar(0, 255, 0), 2, 1);
		ShowImage(pictureBox1, frame);

	}
	private: System::Void button1_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
		//(e->Button == System::Windows::Forms::MouseButtons::Left)


	}

	private: System::Void MyForm_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
		MessageBox::Show(e->KeyCode.ToString());
	}
	private: System::Void MyForm_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
		int a = 99;
	}
	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
		timer1->Start();

	}


	};
}
