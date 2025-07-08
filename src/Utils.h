#pragma once
#include <vector>

class Utils {
public:
	static MpBoolean IsNoteType(MpInteger type, std::vector<MpInteger>& types) {
		for (const auto& t : types) {
			if (type == t) {
				return true;
			}
		}
		return false;
	}
};