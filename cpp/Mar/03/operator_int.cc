/*
** @file    typecast.cc
** @author  jaygong(gongjian0309@gmail.com)
** @date    2016-03-02 23:45:24
*/
 
#include <iostream>

using std::endl;
using std::cout;
using std::ostream;

class Point;
class AnotherPoint
{
	friend ostream &operator<<(ostream &os, const AnotherPoint &rhs);
	friend class Point;
public:
	AnotherPoint(double dx = 0, double dy = 0)
	: _dx(dx)
	, _dy(dy)
	{
		cout << "AnotherPoint(double, double)" <<endl;
	}

private:
	double _dx;
	double _dy;
};

class Point
{
	friend ostream &operator<<(ostream &os, const Point &rhs);
public:
	Point(int ix = 0, int iy = 0)
	: _ix(ix)
	, _iy(iy)
	{
		cout << "Point(int, int)" <<endl;
	}
	Point(AnotherPoint ap)
	: _ix(ap._dx)
	, _iy(ap._dy)
	{
		cout << "Point(AnotherPoint)" << endl;
	}
	//类型转换函数只能以成员函数的形式存在
	//没有函数参数 没有返回值 但是必须以传值的方式return目标类型的变量
	//类型转换的应用
	operator int()
	{
		return _ix;
	}
	operator double()
	{
		return _ix * _iy;
	}
	operator AnotherPoint()
	{
		return AnotherPoint(_ix, _iy);
	}

	
private:
	int _ix;
	int _iy;
};

ostream &operator<<(ostream &os, const AnotherPoint &rhs)
{
	os << "(" << rhs._dx << "," << rhs._dy << ")";
	return os;
}

ostream &operator<<(ostream &os, const Point &rhs)
{
	os << "(" << rhs._ix << "," << rhs._iy << ")";
	return os;
}

int main(void)
{
	Point pt1(4, 5);
	cout << pt1 << endl;
	int x1 = pt1; //在直观思维里边 这种语句不能走通 
				  //在原则上来说，不要轻易使
	cout << "x1 = " << x1 << endl;
	return 0;
}
