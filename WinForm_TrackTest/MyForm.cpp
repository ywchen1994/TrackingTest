#include "MyForm.h"

using namespace System;
using namespace WinForm_TrackTest;
[STAThread]
int main(cli::array<System::String^> ^args)
{
	// 建立任何控制項之前，先啟用 Windows XP 視覺化效果
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	// 建立主視窗並執行
	Application::Run(gcnew MyForm());
	return 0;
}