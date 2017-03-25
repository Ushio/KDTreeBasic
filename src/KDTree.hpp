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
		std::vector<T> points;
		float border = 0.0f;
		int axis = -1;
		bool isLeaf = false;

		std::unique_ptr<KDNode<T>> lhs;
		std::unique_ptr<KDNode<T>> rhs;
	};

	template <class T>
	static inline std::unique_ptr<KDNode<T>> build_tree(const std::vector<T> &points, int depth, int max_elements, Xor &xor_random) {
		int axis = depth % traits::access<T>::DIM;
		if (points.size() <= max_elements) {
			std::unique_ptr<KDNode<T>> leaf(new KDNode<T>());
			leaf->points = points;
			leaf->isLeaf = true;
			return leaf;
		}

		double samples[10];
		int sampleCount = std::min((int)points.size(), (int)sizeof(samples) / (int)sizeof(samples[0]));
		for (int i = 0; i < sampleCount; ++i) {
			samples[i] = traits::access<T>::get(points[xor_random.generate() % sampleCount], axis);
		}
		int samples_mid = (sampleCount - 1) >> 1;
		std::nth_element(samples, samples + samples_mid, samples + sampleCount);
		double heuristic = samples[samples_mid];

		std::unique_ptr<KDNode<T>> node(new KDNode<T>());
		node->border = heuristic;
		node->axis = axis;

		std::vector<T> lhs;
		std::vector<T> rhs;

		for (int i = 0; i < points.size(); ++i) {
			if (traits::access<T>::get(points[i], axis) < node->border) {
				lhs.emplace_back(points[i]);
			}
			else {
				rhs.emplace_back(points[i]);
			}
		}

		node->lhs = build_tree<T>(lhs, depth + 1, max_elements, xor_random);
		node->rhs = build_tree<T>(rhs, depth + 1, max_elements, xor_random);

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
			for (int i = 0; i < node->points.size(); ++i) {
				auto node_point = node->points[i];
				double distanceSq = 0.0;
				for (int j = 0; j < traits::access<T>::DIM; ++j) {
					double x = node_point[j] - origin[j];
					distanceSq += x * x;
				}

				if (distanceSq < radiusSq) {
					func(node->points[i]);
				}
			}
		}
	}

	template <class T>
	class KDTree {
	public:
		KDTree(const std::vector<T> &points) {
			Xor xor_random;
			node = build_tree<T>(points, 0, 5, xor_random);
		}

		template <class F, class P>
		void query(F &func, P origin, double radius) {
			query_tree(func, node, origin, radius, radius * radius);
		}
		std::unique_ptr<KDNode<T>> node;
	};
}
