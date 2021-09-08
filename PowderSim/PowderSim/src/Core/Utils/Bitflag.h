#pragma once


namespace powd::utils
{
	template <typename T>
	class Bitflag
	{
	public:
		T value;

		Bitflag(T val) : value(val) {}

		T GetValue(T mask)
		{
			return (value & mask);
		}
		T SetValue(T val, T mask)
		{
			T untouched = value & (~mask);
			value = untouched | val;

		}
		bool TestMask(T mask)
		{
			return (value & mask) == mask;
		}


		operator T() { return value; }
	};
}
