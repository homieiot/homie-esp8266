cpplint:
	cpplint --repository=. --recursive --filter=-whitespace/line_length,-legal/copyright,-runtime/printf,-build/include,-build/namespace,-runtime/int ./src
.PHONY: cpplint
