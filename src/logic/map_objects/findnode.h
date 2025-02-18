/*
 * Copyright (C) 2008-2023 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_MAP_OBJECTS_FINDNODE_H
#define WL_LOGIC_MAP_OBJECTS_FINDNODE_H

#include <string>

#include "logic/widelands.h"

namespace Widelands {

enum class AnimalBreedable { kDefault, kAnimalFull };

class EditorGameBase;
struct FCoords;

struct FindNode {
private:
	struct BaseCapsule {
		BaseCapsule() = default;
		virtual ~BaseCapsule() = default;

		void addref() {
			++refcount;
		}
		void deref() {
			if (--refcount == 0) {
				delete this;
			}
		}
		[[nodiscard]] virtual bool accept(const EditorGameBase&, const FCoords& coord) const = 0;

		int refcount{1};
	};
	template <typename T> struct Capsule : public BaseCapsule {
		Capsule(const T& init_op) : op(init_op) {  // NOLINT allow implicit conversion
		}
		[[nodiscard]] bool accept(const EditorGameBase& map, const FCoords& coord) const override {
			return op.accept(map, coord);
		}

		const T op;
	};

	BaseCapsule* capsule;

public:
	FindNode(const FindNode& o) {
		capsule = o.capsule;
		capsule->addref();
	}
	~FindNode() {
		capsule->deref();
		capsule = nullptr;
	}
	FindNode& operator=(const FindNode& o) {
		if (&o == this) {
			return *this;
		}
		capsule->deref();
		capsule = o.capsule;
		capsule->addref();
		return *this;
	}

	template <typename T> FindNode(const T& op) {  // NOLINT allow implicit conversion
		capsule = new Capsule<T>(op);
	}

	// Return true if this node should be returned by find_fields()
	[[nodiscard]] bool accept(const EditorGameBase& map, const FCoords& coord) const {
		return capsule->accept(map, coord);
	}
};

struct FindNodeCaps {
	explicit FindNodeCaps(uint8_t init_mincaps) : mincaps(init_mincaps) {
	}

	[[nodiscard]] bool accept(const EditorGameBase&, const FCoords&) const;

private:
	uint8_t mincaps;
};

/// Accepts a node if it is accepted by all subfunctors.
struct FindNodeAnd {
	FindNodeAnd() = default;

	void add(const FindNode&, bool negate = false);

	[[nodiscard]] bool accept(const EditorGameBase&, const FCoords&) const;

private:
	struct Subfunctor {
		bool negate;
		FindNode findfield;

		Subfunctor(const FindNode&, bool init_negate);
	};

	std::vector<Subfunctor> subfunctors;
};

/// Accepts a node based on what can be built there.
struct FindNodeSize {
	enum Size {
		sizeAny = 0,  //  any field not occupied by a robust immovable
		sizeBuild,    //  any field we can build on (flag or building)
		sizeSmall,    //  at least small size
		sizeMedium,
		sizeBig,
		sizeMine,  //  can build a mine on this field
		sizePort,  //  can build a port on this field
		sizeSwim   //  coast
	};

	explicit FindNodeSize(Size init_size) : size(init_size) {
	}

	[[nodiscard]] bool accept(const EditorGameBase&, const FCoords&) const;

private:
	Size size;
};

/// Accepts a node based on the size of the immovable there (if any).
struct FindNodeImmovableSize {
	enum { sizeNone = 1 << 0, sizeSmall = 1 << 1, sizeMedium = 1 << 2, sizeBig = 1 << 3 };

	explicit FindNodeImmovableSize(uint32_t init_sizes) : sizes(init_sizes) {
	}

	[[nodiscard]] bool accept(const EditorGameBase&, const FCoords&) const;

private:
	uint32_t sizes;
};

/// Accepts a node if it has an immovable with a given attribute.
struct FindNodeImmovableAttribute {
	explicit FindNodeImmovableAttribute(uint32_t attrib) : attribute(attrib) {
	}

	[[nodiscard]] bool accept(const EditorGameBase&, const FCoords&) const;

private:
	uint32_t attribute;
};

/// Accepts a node if it has at least one of the given resource.
struct FindNodeResource {
	explicit FindNodeResource(DescriptionIndex res) : resource(res) {
	}

	[[nodiscard]] bool accept(const EditorGameBase&, const FCoords&) const;

private:
	DescriptionIndex resource;
};

/// Accepts a node if it has the given resource type and remaining capacity.
/// If 'br' == AnimalBreedable::kAnimalFull, only accepts the node if it is full
struct FindNodeResourceBreedable {
	explicit FindNodeResourceBreedable(DescriptionIndex res,
	                                   AnimalBreedable br = AnimalBreedable::kDefault)
	   : resource(res), strictness(br) {
	}

	[[nodiscard]] bool accept(const EditorGameBase&, const FCoords&) const;

private:
	DescriptionIndex resource;
	AnimalBreedable strictness;
};

/// Accepts a node where at least 1 adjacent triangle has enhancable terrain
struct FindNodeTerraform {
	explicit FindNodeTerraform(const std::string& c) : category_(c) {
	}
	[[nodiscard]] bool accept(const EditorGameBase&, const FCoords&) const;

	const std::string category_;
};

/// Accepts a node if it is a shore node in the sense that it is walkable
/// and has at least one neighbouring field that is swimmable
struct FindNodeShore {
	explicit FindNodeShore(uint16_t f = 1) : min_fields(f) {
	}

	[[nodiscard]] bool accept(const EditorGameBase&, const FCoords&) const;

private:
	// Minimal number of reachable swimmable fields. 1 is minimum for this to be considered "shore"
	uint16_t min_fields;
};

/** Accepts a node if and only if a ferry can reach it or depart from there. */
struct FindNodeFerry {
	explicit FindNodeFerry(PlayerNumber p) : player_who_needs_ferry_(p) {
	}

	[[nodiscard]] bool accept(const EditorGameBase& egbase, const FCoords& coords) const;

private:
	const PlayerNumber player_who_needs_ferry_;
};

}  // namespace Widelands
#endif  // end of include guard: WL_LOGIC_FINDNODE_H
