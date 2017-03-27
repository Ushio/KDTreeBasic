#pragma once

#include <memory>
#include <algorithm>


namespace kd {
	struct Xor {
		Xor() {

		}
		Xor(uint32_t seed) {
			_y = std::max(seed, 1u);
		}

		// 0 <= x <= 0x7FFFFFFF
		uint32_t generate() {
			_y = _y ^ (_y << 13); _y = _y ^ (_y >> 17);
			uint32_t value = _y = _y ^ (_y << 5); // 1 ~ 0xFFFFFFFF(4294967295
			return value >> 1;
		}
		// 0.0 <= x < 1.0
		double uniform() {
			return double(generate()) / double(0x80000000);
		}
		double uniform(double a, double b) {
			return a + (b - a) * uniform();
		}
	public:
		uint32_t _y = 2463534242;
	};

	namespace traits
	{
		template <typename T>
		struct access {};
	}

	template <class T>
	struct KDNode {
		T *points_beg = nullptr;
		T *points_end = nullptr;
		float border = 0.0f;
		int axis = -1;
		bool isLeaf = false;

		std::unique_ptr<KDNode<T>> lhs;
		std::unique_ptr<KDNode<T>> rhs;
	};

	template <class T>
	static inline std::unique_ptr<KDNode<T>> build_tree(T *points_beg, T *points_end, int depth, int max_elements, Xor &xor_random) {
		int axis = depth % traits::access<T>::DIM;
		int pointCount = (int)(points_end - points_beg);
		if (pointCount <= max_elements) {
			std::unique_ptr<KDNode<T>> leaf(new KDNode<T>());
			leaf->points_beg = points_beg;
			leaf->points_end = points_end;
			leaf->isLeaf = true;
			return leaf;
		}

		double samples[10];
		int sampleCount = std::min(pointCount, (int)sizeof(samples) / (int)sizeof(samples[0]));
		for (int i = 0; i < sampleCount; ++i) {
			samples[i] = traits::access<T>::get(points_beg[xor_random.generate() % sampleCount], axis);
		}
		int samples_mid = (sampleCount - 1) >> 1;
		std::nth_element(samples, samples + samples_mid, samples + sampleCount);
		double heuristic = samples[samples_mid];

		T *points_lhs = points_beg;
		T *points_rhs = points_end - 1;

		while (true) {
			while (traits::access<T>::get(*points_lhs, axis) < heuristic) {
				points_lhs++;
				// assert(points_lhs < points_end);
			}
			while (heuristic < traits::access<T>::get(*points_rhs, axis)) {
				points_rhs--;
				// assert(points_beg <= points_rhs);
			}

			if (points_rhs < points_lhs) {
				break;
			}
			if (points_rhs == points_lhs) {
				if (points_lhs == points_beg) {
					points_lhs++;
				}
				else {
					points_rhs--;
				}
				break;
			}

			std::swap(*points_lhs, *points_rhs);
			points_lhs++;
			points_rhs--;
		}

		std::vector<double> inputs(pointCount);
		{
			int di = 0;
			for (auto it = points_beg; it != points_end; ++it) {
				inputs[di] = traits::access<T>::get(*it, axis);
				di++;
			}
		}

		std::unique_ptr<KDNode<T>> node(new KDNode<T>());
		node->border = heuristic;
		node->axis = axis;

		//// 検算
		//int count = 0;
		//for (auto it = points_beg; it != points_lhs; ++it) {
		//	assert(traits::access<T>::get(*it, axis) <= heuristic);
		//	count++;
		//}
		//for (auto it = points_lhs; it != points_end; ++it) {
		//	assert(heuristic <= traits::access<T>::get(*it, axis));
		//	count++;
		//}
		//assert(count == pointCount);

		node->lhs = build_tree<T>(points_beg, points_lhs, depth + 1, max_elements, xor_random);
		node->rhs = build_tree<T>(points_lhs, points_end, depth + 1, max_elements, xor_random);

		return node;
	}

	template <class T, class F, class P>
	void query_tree(F &func, const std::unique_ptr<kd::KDNode<T>> &node, P origin, double radius, double radiusSq) {
		if (node->isLeaf == false) {
			if (origin[node->axis] < node->border + radius) {
				query_tree(func, node->lhs, origin, radius, radiusSq);
			}
			if (node->border - radius < origin[node->axis]) {
				query_tree(func, node->rhs, origin, radius, radiusSq);
			}
		}
		else {
			for (auto it = node->points_beg; it != node->points_end; ++it) {
				auto node_point = *it;
				double distanceSq = 0.0;
				for (int j = 0; j < traits::access<T>::DIM; ++j) {
					double x = traits::access<T>::get(node_point, j) - origin[j];
					distanceSq += x * x;
				}

				if (distanceSq < radiusSq) {
					func(*it);
				}
			}
		}
	}

	template <class T>
	class KDTree {
	public:
		KDTree(const std::vector<T> &points) {
			_points = points;
			Xor xor_random;
			node = build_tree<T>(_points.data(), _points.data() + _points.size(), 0, 5, xor_random);
		}

		template <class F, class P>
		void query(F &func, P origin, double radius) const {
			query_tree(func, node, origin, radius, radius * radius);
		}
		std::unique_ptr<KDNode<T>> node;
		std::vector<T> _points;
	};
}
