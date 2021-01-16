#include "../Common/ImportsExports.h"


#include <unordered_set>

std::unordered_set<unsigned> powdsToKill;
std::unordered_set<unsigned long long> powdsToMake;


EXPORT void PreTick()
{
	powdsToKill.clear();
	powdsToMake.clear();
}
EXPORT void PostTick()
{
	for (unsigned powd : powdsToKill)
	{
		imports.RemovePowder(powd);
	}
	for (unsigned long long powd : powdsToMake)
	{
		unsigned tmp;
		unsigned x = (unsigned)(powd >> 32), y = (unsigned)powd;
		imports.CreateNewPowder("core:test", { x, y }, tmp);
	}
}

BEHAVIOUR_FUNCTION(TestBehaviourOne)
{
	unsigned mainLivingNeighbours = 0;
	unsigned livingNeighbours;

	vec2 position = imports.GetPowderPosition(powder);

	for (unsigned x = position.x - 1; x <= position.x + 1; x++)
	{
		for (unsigned y = position.y - 1; y <= position.y + 1; y++)
		{
			vec2 peekPos = { x, y };
			if (peekPos == position)
				continue;
			livingNeighbours = 0;

			for (unsigned ix = peekPos.x - 1; ix <= peekPos.x + 1; ix++)
			{
				for (unsigned iy = peekPos.y - 1; iy <= peekPos.y + 1; iy++)
				{
					if (ix == peekPos.x && iy == peekPos.y)
						continue;
					if (imports.GetPowder({ ix, iy }) != nullPowder)
						livingNeighbours++;
				}
			}

			unsigned peekPowd = imports.GetPowder(peekPos);

			if (peekPowd == nullPowder)
			{
				if (livingNeighbours == 3)
				{
					powdsToMake.insert(((unsigned long long)peekPos.x << 32) + peekPos.y);
				}
			}
			else
			{
				mainLivingNeighbours++;
				if (livingNeighbours != 2 && livingNeighbours != 3)
				{
					powdsToKill.insert(peekPowd);
				}
			}
		}
	}

	if (mainLivingNeighbours != 2 && mainLivingNeighbours != 3)
	{
		powdsToKill.insert(powder);
	}

	return true;
}
