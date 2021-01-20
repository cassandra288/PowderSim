#pragma once

#include <vector>
#include <mutex>
#include <assert.h>
#include <unordered_set>


namespace powd::utils
{
	template<typename T>
	class DualBlockArray
	{
	public:
#undef max
		static constexpr unsigned reservedVal = std::numeric_limits<unsigned>::max();
	private:
		std::mutex mut;
		unsigned arrBlockOneSize = 0;
		std::vector<T> dataVec = {};
		std::vector<unsigned> indexToData = {};
		std::unordered_set<unsigned> indexToDataGaps;
		std::vector<unsigned> dataToIndex = {};


		void SwapData(unsigned _i1, unsigned _i2)
		{
			std::swap(dataVec[_i1], dataVec[_i2]);
			std::swap(dataToIndex[_i1], dataToIndex[_i2]);
			std::swap(indexToData[MapIndex(_i1)], indexToData[MapIndex(_i2)]);
		}
		unsigned AddData(const T& _val, unsigned _i)
		{
			dataVec[_i] = _val;
			unsigned mapIndex = indexToData.size();
			if (indexToDataGaps.size() > 0)
			{
				mapIndex = *indexToDataGaps.begin();
				indexToDataGaps.erase(mapIndex);
			}
			else
			{
				indexToData.resize(indexToData.size() + 1);
			}

			dataToIndex[_i] = mapIndex;
			indexToData[mapIndex] = _i;

			return mapIndex;
		}

		unsigned DataIndex(unsigned _i)
		{
			assert(_i < indexToData.size());
			return indexToData[_i];
		}
		unsigned MapIndex(unsigned _i)
		{
			assert(_i < dataToIndex.size());
			return dataToIndex[_i];
		}


	public:
#pragma region CTOR
		DualBlockArray() {}
#pragma endregion
#pragma region COPY CTOR/=
		DualBlockArray(const DualBlockArray& _o)
		{
			std::lock_guard<std::mutex> lk1(mut);
			std::lock_guard<std::mutex> lk2(_o.mut);

			arrBlockOneSize = _o.arrBlockOneSize;
			dataVec = _o.dataVec;
			indexToData = _o.indexToData;
			indexToDataGaps = _o.indexToDataGaps;
			dataToIndex = _o.dataToIndex;
		}
		DualBlockArray& operator =(const DualBlockArray& _o)
		{
			std::lock_guard<std::mutex> lk1(mut);
			std::lock_guard<std::mutex> lk2(_o.mut);

			arrBlockOneSize = _o.arrBlockOneSize;
			dataVec = _o.dataVec;
			indexToData = _o.indexToData;
			indexToDataGaps = _o.indexToDataGaps;
			dataToIndex = _o.dataToIndex;

			return *this;
		}
#pragma endregion
#pragma region MOVE CTOR/=
		DualBlockArray(DualBlockArray&& _o)
		{
			std::lock_guard<std::mutex> lk1(mut);
			std::lock_guard<std::mutex> lk2(_o.mut);

			InternalSwap(*this, _o);
		}
		DualBlockArray& operator =(DualBlockArray&& _o)
		{
			std::lock_guard<std::mutex> lk1(mut);
			std::lock_guard<std::mutex> lk2(_o.mut);

			InternalSwap(*this, _o);

			return *this;
		}
#pragma endregion

#pragma region Element Access
		T* data()
		{
			std::lock_guard<std::mutex> lk(mut);
			return dataVec.data();
		}

		T& get(unsigned _i)
		{
			std::lock_guard<std::mutex> lk(mut);
			return dataVec[DataIndex(_i)];
		}
		T& operator [](unsigned _i)
		{
			std::lock_guard<std::mutex> lk(mut);
			return dataVec[DataIndex(_i)];
		}

		bool contains(unsigned _i)
		{
			return _i < indexToData.size();
		}
#pragma endregion
#pragma region Capacity
		unsigned size()
		{
			std::lock_guard<std::mutex> lk(mut);
			return dataVec.size();
		}
		unsigned blockOneSize()
		{
			std::lock_guard<std::mutex> lk(mut);
			return arrBlockOneSize;
		}
		unsigned blockTwoSize()
		{
			std::lock_guard<std::mutex> lk(mut);
			return dataVec.size() - arrBlockOneSize;
		}
		bool empty()
		{
			std::lock_guard<std::mutex> lk(mut);
			return dataVec.empty();
		}
		bool inBlockOne(unsigned _i)
		{
			std::lock_guard<std::mutex> lk(mut);
			return _i < arrBlockOneSize;
		}
		bool inBlockTwo(unsigned _i)
		{
			std::lock_guard<std::mutex> lk(mut);
			return _i >= arrBlockOneSize;
		}
#pragma endregion
#pragma region Modifiers
		private:
			void InternalResize(unsigned _size)
			{
				assert(_size < reservedVal);
				dataVec.resize(_size);

				unsigned _oldSize = dataToIndex.size();
				dataToIndex.resize(_size);
				for (unsigned i = _oldSize; i < dataToIndex.size(); i++)
				{
					dataToIndex[i] = i;
				}
			}
		public:
			void Clear()
			{
				std::lock_guard<std::mutex> lk(mut);
				InternalResize(0);
			}

			unsigned Insert(const T& _value, bool _blockOne = true)
			{
				std::lock_guard<std::mutex> lk(mut);
				InternalResize(dataVec.size() + 1);

				unsigned i = AddData(_value, dataVec.size() - 1);
				if (_blockOne)
					SwapData(dataVec.size() - 1, arrBlockOneSize++);

				return i;
			}
			void RemoveAt(unsigned _i)
			{
				std::lock_guard<std::mutex> lk(mut);
				assert(_i < indexToData.size());
				_i = DataIndex(_i);
				assert(_i < dataVec.size());

				if (_i < arrBlockOneSize)
				{
					SwapData(_i, --arrBlockOneSize); // move it to the edge of block one
					_i = arrBlockOneSize;
				}

				if (_i < dataVec.size() - 1)
				{
					SwapData(_i, dataVec.size() - 1); // move to end of array
				}

				_i = MapIndex(dataVec.size() - 1);
				if (_i == indexToData.size() - 1)
				{
					unsigned shrinkCount = 0;
					bool entryNull = true;
					while (entryNull)
					{
						shrinkCount++;
						entryNull = indexToDataGaps.find(_i - shrinkCount) != indexToDataGaps.end();
						if (entryNull)
							indexToDataGaps.erase(_i - shrinkCount);
					}
					indexToData.resize(indexToData.size() - shrinkCount);
				}
				else
				{
					indexToDataGaps.insert(_i);
				}

				InternalResize(dataVec.size() - 1);
			}

			void MoveToBlock1(unsigned _i)
			{
				SwapData(DataIndex(_i), arrBlockOneSize++); // move it to directly infront of the first block
			}
			void MoveToBlock2(unsigned _i)
			{
				SwapData(DataIndex(_i), --arrBlockOneSize); // move it to the end of the first block
			}
#pragma endregion


	
	private:
		static void InternalSwap(DualBlockArray& _left, DualBlockArray& _right)
		{
			using std::swap;

			swap(_left.arrBlockOneSize, _right.arrBlockOneSize);
			swap(_left.dataVec, _right.dataVec);
			swap(_left.indexToData, _right.indexToData);
			swap(_left.indexToDataGaps, _right.indexToDataGaps);
			swap(_left.dataToIndex, _right.dataToIndex);
		}
	public:
		friend void swap(DualBlockArray& _left, DualBlockArray& _right)
		{
			std::lock_guard<std::mutex> lk1(_left.mut);
			std::lock_guard<std::mutex> lk2(_right.mut);

			InternalSwap(_left, _right);
		}
	};
}
