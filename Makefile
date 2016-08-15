test:
	cpplint --repository=. --recursive --filter=-whitespace/line_length,-legal/copyright,-runtime/printf,-build/include,-build/namespaces ./src

.PHONY: test
