#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <string>
#include <utility>
#include <map>

using namespace std;

#define M 131072
#define root ((M) * 2 - 1)
#define INF (2e9 + 7)

int N, K, l, r;
int i, j;
pair <int, int> arr[root + 1];

void build() {
	for (int i = M + 1; i <= root; i++) {
		int r = (i - M) * 2;
		int l = r - 1;

		if (arr[l].first > arr[r].first) {
			arr[i] = make_pair(arr[l].first, arr[l].second);
		} else {
			arr[i] = make_pair(arr[r].first, arr[r].second);
		}	
	}
}

pair <int, int> getMax(int l, int r) {
	pair <int, int> ans (-INF, 0);

	if (l % 2 == 0) {
		if (arr[l].first > ans.first) {
			ans = make_pair(arr[l].first, arr[l].second);
		}

		l++;
	}

	if (r % 2 == 1) {
		if (arr[r].first > ans.first) {
			ans = make_pair(arr[r].first, arr[r].second);
		}

		r--;
	}

	if (l < r) {
		pair <int, int> temp (getMax(M + (l / 2) + (l % 2), M + (r / 2) + (r % 2)));

		if (temp.first > ans.first) {
			ans = make_pair(temp.first, temp.second);
		}
	}

	return ans;
}

int main() {
	
	scanf("%i", &N);

	for (i = 1; i <= N; i++) {
		scanf("%i", &arr[i].first);
		                  
		arr[i].second = i;	
	}

	for (i = N + 1; i <= M; i++) {
		arr[i] = make_pair(-INF, i);
	}

	build();

	scanf("%i", &K);

	for (i = 0; i < K; i++) {
		scanf("%i %i", &l, &r);

		pair <int, int> ans (getMax(l, r));

		printf("%i %i\n", ans.first, ans.second);
	}

	return 0;
}
