#ifndef FILTER_HPP
#define FILTER_HPP
#include <matrix/Matrix.hpp>
constexpr uint8_t filterOrder = 17U;
class fpfilter
{
public:
	fpfilter(const float val = 0): result(val)
	{
		/*Initialize Vector values*/
		for(int m=0;m<filterOrder;m++)
		{
			values(m) = val;
		}
	}
	void new_sample(matrix::Vector<float,filterOrder>& b,const float& val)
	{
		result = 0.0f;
		/*Shift delay line and put new value in the delay line*/
		for(size_t t=0; t<(filterOrder-1); t++)
		{
			values(t) = values(t+1);
		}
		values(filterOrder-1) = val;
		/*Perform FIR algorithm*/
		for(size_t t=0; t<filterOrder; t++)
		{
			result = result + (b(t) * values(t));
		}
	}
	const float& get_result() const
	{
		return result;
	}
private:
	float result;
	matrix::Vector<float,17U> values;
};
#endif
