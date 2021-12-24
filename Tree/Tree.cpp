#include <iostream>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <random>
#include <fstream>
#include <atomic>
#include <windows.h>
#include <WinBase.h>
#define INF 0x7fffffff
using namespace std;
using namespace literals::chrono_literals;

//定义颜色代码类型
enum class color_type : unsigned char {};
constexpr color_type operator "" _c(unsigned long long int v)
{
	return color_type(v);
}
//格式化输出颜色代码
ostream& operator<<(ostream& os, color_type val)
{
	return os << "\033[38;5;" << static_cast<int>(val) << 'm';
}

//叶子渐变
struct green_shading {
private:
	//叶子的几种不同绿色
	constexpr static color_type GreenColors[] = { 22_c, 22_c, 28_c, 28_c, 34_c };
	int dark = 0;
public:
	color_type color() const
	{
		//输出当前叶子的颜色(此种方式防止out of range)
		return *(min(begin(GreenColors) + dark, end(GreenColors) - 1));
	}
	//叶子变亮
	void increase()
	{
		++dark;
	}
	//叶子颜色重置(最亮)
	void reset()
	{
		dark = 0;
	}
};
//灯的颜色(五颜六色)
struct lamps {
private:
	// 颜色代码网站： https://misc.flogisoft.com/bash/tip_colors_and_formatting
	static constexpr color_type Pretty_Colors[] = {
			1_c, 9_c, 11_c, 15_c, 45_c, 87_c, 118_c, 154_c, 155_c, 165_c, 193_c, 196_c, 198_c, 208_c, 226_c, 15_c
	};

	atomic<bool> stopped = false;
	atomic<size_t> mode{ 0 };
	//三种灯光状态
	static constexpr size_t max_state = 3;
	mt19937 rand{ random_device{}() };
	color_type col = 0_c;

	//随机颜色
	void new_color()
	{
		sample(begin(Pretty_Colors), end(Pretty_Colors), &col, 1, rand);
	}

public:
	void operator()(char c)
	{
		//第一种模式:每个灯随机颜色
		if (mode % max_state == 0)
			new_color();
		cout << col << c;
	}

	void end_cycle()
	{
		//第二种模式:整体的灯随机颜色
		if (mode % max_state == 1)
		{
			new_color();
		}
		//第三种模式:整体灯光指定颜色范围变幻
		else if (mode % max_state == 2)
		{
			auto i = static_cast<int>(col);
			++i;

			i %= 226;
			i = clamp(i, 154, 226);

			//i %= 202;
			//i = clamp(i, 196, 202);
			col = static_cast<color_type>(i);
		}
	}
	//改变灯光模式
	void change_mode()
	{
		++mode;
	}
	

	//设置退出程序状态
	bool was_stopped()
	{
		return stopped;
	}
	void stop()
	{
		stopped = true;
	}

};


//移动光标
void gotoxy(HANDLE h_out, int x, int y)
{
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(h_out, pos);
}
//将圣诞树置于终端中央输出
void to_middle(int width)
{
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO bInfo;
	GetConsoleScreenBufferInfo(hOutput, &bInfo);
	int dwSizeX = bInfo.dwSize.X, dwSizey = bInfo.dwSize.Y;//获取控制台屏幕缓冲区大小
	int x = dwSizeX / 2 - width / 2;
	int y=bInfo.dwCursorPosition.Y;
	gotoxy(hOutput, x, y);
}
//打印分割线
void split(int width)
{
	to_middle(width);
	for (int i = 1; i <= width;i++)
		cout << 97_c << '-';
	return;
}
//打印圣诞树
void format_tree(int tree_size)
{
	srand((unsigned)time(0));
	if (tree_size % 2==0)
		tree_size++;
	lamps lamp;
	//加入线程,监控用户输入
	thread t([&lamp]()
		{
			char input;
			//输入q停止程序
			while (cin >> input)
			{
				if (input == 'q')
				{
					lamp.stop();
					break;
				}
				//输入其他键改变灯光模式
				lamp.change_mode();
			}
		});
	for (;;)
	{
		//停止打印
		if (lamp.was_stopped())
			break;
		green_shading g;//控制叶子颜色渐变
		char c;
		int count_tree_char = 0;//圣诞树字符个数
		int blank_space = 0;//叶子左边空白长度
		int tree_space = 0;//叶子一排长度
		int block = 0;//有几堆叶子
		int block_height = 0;//一堆叶子高度
		int block_width = 0;//一堆叶子的最大长度
		int pre_width = INF;//上一排叶子长度
		if (tree_size <= 13)
		{
			block = 1;
			block_height = (tree_size + 1) / 2;
			block_width = tree_size;
		}
		else
		{
			block = tree_size / 13 + 1;
			block_height = (tree_size + 1) / 2 / block;
			block_width = block_height * 2 - 1;
		}
		//打印分割线
		to_middle(tree_size);
		split(tree_size);
		cout << endl << endl;
		bool change = 1;
		tree_space = block_width - block_height * 2 + 2;
		for (int i = 1; i <= block; i++)
		{
			for (int j = 1; j <= block_height; j++)
			{
				to_middle(tree_size);
				blank_space = tree_size / 2 - tree_space / 2;
				//打印左边空白区域
				for (int k = 1; k <= blank_space; k++)
					cout << " ";
				//一堆叶子从上到下颜色依次变亮(有层次感)
				if (pre_width < tree_space)
					g.increase();
				else
					g.reset();
				for (int k = 1; k <= tree_space; k++)
				{
					//打印圣诞树顶部
					if (i == 1 && j == 1)
						cout << 220_c << "X--..";
					else
					{
						//不在圣诞树边缘打印灯光
						if (k != 1 && k != tree_space)
						{
							//随机打印圣诞树灯光
							if (count_tree_char % ((rand() % 15) + 5) == 0)
							{
								//随机生成大灯和小灯
								if (rand() % 10 == 0)
									c = '0';
								else
									c = 'o';
								lamp(c);
							}
							else
								cout << g.color() << "*";
						}
						//打印叶子
						else
						{
							cout << g.color() << "*";
						}
						count_tree_char++;
					}
				}
				pre_width = tree_space;
				tree_space += 2;
				cout << 0_c << endl;

			}
			block_height++;
			block_width = tree_space - 2;
			tree_space = block_width - i * 2;
		}
		int trunk_width = tree_size / 6;//树干宽度
		int trunk_height = trunk_width * 1.1;//树干高度
		for (int i = 1; i <= trunk_height; i++)
		{
			to_middle(tree_size);
			int blank_space = tree_size / 2 - trunk_width / 2;
			for (int j = 1; j <= blank_space; j++)
				cout << " ";
			for (int j = 1; j <= trunk_width; j++)
				cout << 94_c << "#";
			cout << endl;
		}
		cout << endl << endl;
		to_middle(31);
		cout << 196_c << "|             小若             |\n";
		to_middle(31);
		cout << 196_c << "|  --> Merry Christmas :) <--  |\n";
		cout << endl << endl;
		to_middle(tree_size);
		split(tree_size);
		cout << 0_c << endl;
		//判断是否灯光模式是否改变
		lamp.end_cycle();
		//打印圣诞树间隔1.5s
		this_thread::sleep_for(1500ms);
	}
	t.join();
	cout << 15_c;
}

int main()
{
	system("cls");
	while (1)
	{
		cout << "请输入\"start\"开始程序" << endl;
		cout << "程序运行后输入\"q键\"停止程序,输入其他任意键更换灯光模式" << endl;
		string input;
		while (cin >> input)
		{
			if (input == "start")
			{
				cout << "请输入圣诞树宽度(不小于11的数字)" << endl;
				int tree_max_width;
				while (cin >> tree_max_width)
				{
					if (tree_max_width >= 11)
					{
						format_tree(tree_max_width);
						break;
					}
					else
					{
						cout << "输入错误！" << endl;
						cout << "请重新输入圣诞树宽度(不小于11的数字)" << endl;
					}
				}
				break;
			}
			else
			{
				cout << "输入错误！" << endl;
				cout << "请输入\"start\"开始程序！！！" << endl;
			}
		}
		system("cls");
	}
	return 0;
}