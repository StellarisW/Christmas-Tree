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

//������ɫ��������
enum class color_type : unsigned char {};
constexpr color_type operator "" _c(unsigned long long int v)
{
	return color_type(v);
}
//��ʽ�������ɫ����
ostream& operator<<(ostream& os, color_type val)
{
	return os << "\033[38;5;" << static_cast<int>(val) << 'm';
}

//Ҷ�ӽ���
struct green_shading {
private:
	//Ҷ�ӵļ��ֲ�ͬ��ɫ
	constexpr static color_type GreenColors[] = { 22_c, 22_c, 28_c, 28_c, 34_c };
	int dark = 0;
public:
	color_type color() const
	{
		//�����ǰҶ�ӵ���ɫ(���ַ�ʽ��ֹout of range)
		return *(min(begin(GreenColors) + dark, end(GreenColors) - 1));
	}
	//Ҷ�ӱ���
	void increase()
	{
		++dark;
	}
	//Ҷ����ɫ����(����)
	void reset()
	{
		dark = 0;
	}
};
//�Ƶ���ɫ(������ɫ)
struct lamps {
private:
	// ��ɫ������վ�� https://misc.flogisoft.com/bash/tip_colors_and_formatting
	static constexpr color_type Pretty_Colors[] = {
			1_c, 9_c, 11_c, 15_c, 45_c, 87_c, 118_c, 154_c, 155_c, 165_c, 193_c, 196_c, 198_c, 208_c, 226_c, 15_c
	};

	atomic<bool> stopped = false;
	atomic<size_t> mode{ 0 };
	//���ֵƹ�״̬
	static constexpr size_t max_state = 3;
	mt19937 rand{ random_device{}() };
	color_type col = 0_c;

	//�����ɫ
	void new_color()
	{
		sample(begin(Pretty_Colors), end(Pretty_Colors), &col, 1, rand);
	}

public:
	void operator()(char c)
	{
		//��һ��ģʽ:ÿ���������ɫ
		if (mode % max_state == 0)
			new_color();
		cout << col << c;
	}

	void end_cycle()
	{
		//�ڶ���ģʽ:����ĵ������ɫ
		if (mode % max_state == 1)
		{
			new_color();
		}
		//������ģʽ:����ƹ�ָ����ɫ��Χ���
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
	//�ı�ƹ�ģʽ
	void change_mode()
	{
		++mode;
	}
	

	//�����˳�����״̬
	bool was_stopped()
	{
		return stopped;
	}
	void stop()
	{
		stopped = true;
	}

};


//�ƶ����
void gotoxy(HANDLE h_out, int x, int y)
{
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(h_out, pos);
}
//��ʥ���������ն��������
void to_middle(int width)
{
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO bInfo;
	GetConsoleScreenBufferInfo(hOutput, &bInfo);
	int dwSizeX = bInfo.dwSize.X, dwSizey = bInfo.dwSize.Y;//��ȡ����̨��Ļ��������С
	int x = dwSizeX / 2 - width / 2;
	int y=bInfo.dwCursorPosition.Y;
	gotoxy(hOutput, x, y);
}
//��ӡ�ָ���
void split(int width)
{
	to_middle(width);
	for (int i = 1; i <= width;i++)
		cout << 97_c << '-';
	return;
}
//��ӡʥ����
void format_tree(int tree_size)
{
	srand((unsigned)time(0));
	if (tree_size % 2==0)
		tree_size++;
	lamps lamp;
	//�����߳�,����û�����
	thread t([&lamp]()
		{
			char input;
			//����qֹͣ����
			while (cin >> input)
			{
				if (input == 'q')
				{
					lamp.stop();
					break;
				}
				//�����������ı�ƹ�ģʽ
				lamp.change_mode();
			}
		});
	for (;;)
	{
		//ֹͣ��ӡ
		if (lamp.was_stopped())
			break;
		green_shading g;//����Ҷ����ɫ����
		char c;
		int count_tree_char = 0;//ʥ�����ַ�����
		int blank_space = 0;//Ҷ����߿հ׳���
		int tree_space = 0;//Ҷ��һ�ų���
		int block = 0;//�м���Ҷ��
		int block_height = 0;//һ��Ҷ�Ӹ߶�
		int block_width = 0;//һ��Ҷ�ӵ���󳤶�
		int pre_width = INF;//��һ��Ҷ�ӳ���
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
		//��ӡ�ָ���
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
				//��ӡ��߿հ�����
				for (int k = 1; k <= blank_space; k++)
					cout << " ";
				//һ��Ҷ�Ӵ��ϵ�����ɫ���α���(�в�θ�)
				if (pre_width < tree_space)
					g.increase();
				else
					g.reset();
				for (int k = 1; k <= tree_space; k++)
				{
					//��ӡʥ��������
					if (i == 1 && j == 1)
						cout << 220_c << "X--..";
					else
					{
						//����ʥ������Ե��ӡ�ƹ�
						if (k != 1 && k != tree_space)
						{
							//�����ӡʥ�����ƹ�
							if (count_tree_char % ((rand() % 15) + 5) == 0)
							{
								//������ɴ�ƺ�С��
								if (rand() % 10 == 0)
									c = '0';
								else
									c = 'o';
								lamp(c);
							}
							else
								cout << g.color() << "*";
						}
						//��ӡҶ��
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
		int trunk_width = tree_size / 6;//���ɿ��
		int trunk_height = trunk_width * 1.1;//���ɸ߶�
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
		cout << 196_c << "|             С��             |\n";
		to_middle(31);
		cout << 196_c << "|  --> Merry Christmas :) <--  |\n";
		cout << endl << endl;
		to_middle(tree_size);
		split(tree_size);
		cout << 0_c << endl;
		//�ж��Ƿ�ƹ�ģʽ�Ƿ�ı�
		lamp.end_cycle();
		//��ӡʥ�������1.5s
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
		cout << "Please input\"start\"" << endl;
		cout << "When project is running,pressing\"q\"for quit, press any key except 'q' to change lamp mode" << endl;
		string input;
		while (cin >> input)
		{
			if (input == "start")
			{
				cout << "Input width of Christmas tree (not less than 11)" << endl;
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
						cout << "Error!!!" << endl;
						cout << "Please input width of Christmas tree (not less than 11)!!!" << endl;
					}
				}
				break;
			}
			else
			{
				cout << "Error!!!" << endl;
				cout << "Please input\"start\"!!!" << endl;
			}
		}
		system("cls");
	}
	return 0;
}