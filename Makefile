cpplint:
	cpplint --repository=. --recursive --filter=-whitespace/line_length,-legal/copyright,-runtime/printf,-build/include,-build/namespace,-runtime/int,-whitespace/comments,-runtime/threadsafe_fn ./src
.PHONY: cpplint
