#ifndef ADAPTIONCATEGORIES_H_
#define ADAPTIONCATEGORIES_H_

#include "ns3/MySingleton.h"
#include "ns3/DataBasis.h"
#include "ns3/MyConfig.h"

#define SWITCHABLE_PRIORITY_STAGES 2

namespace ns3 {

class AdaptionCategories : public MySingleton<AdaptionCategories> {
	friend class MySingleton<AdaptionCategories>;

private:
	std::vector<double>* categories;
	std::map<int, int> bufferCategory;

public:
	AdaptionCategories() {
		categories = (*DataBasis::Get()->get("dsm/" + MyConfig::Get()->getString("adaption_categories"), "csv", CSV_PER_COLUMN, false))[0];
	}
	virtual ~AdaptionCategories() {}

	std::vector<double>* getCategories() { return categories; }

	int getOnOffCategory(int value) {
		std::map<int, int>::iterator it = bufferCategory.find(value);
		if(it != bufferCategory.end())
			return it->second;
		else {
			int category = 0;
			for(category = 0; category < getNumberWithoutPrios()-1; category++) {
				if(value <= (*categories)[category])
					break;
			}
			bufferCategory.insert(std::pair<int, int>(value, category));
			return category;
		}
	}

	int getOnOffCategoryRating(int category) {
		return (*categories)[category];
	}

	int getNumber() { return categories->size() * SWITCHABLE_PRIORITY_STAGES; }
	int getNumberWithoutPrios() { return categories->size(); }
	int getPrios() { return SWITCHABLE_PRIORITY_STAGES; }
};

}

#endif
