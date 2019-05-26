int func (int a, int b) {
	int sum = 0;
	int iter;
	for (iter = 0; iter < a; iter++) {
		int iter1;
		for (iter1 = 0; iter1 < b; iter1++) {
			sum += iter > iter1?1:0;
		}
	}
	return sum;
}
