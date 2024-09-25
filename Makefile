HANDIN_ARCHIVE=handin.tar.gz

handin:
	@echo "Creating archive $(HANDIN_ARCHIVE) with all .c and .txt files..."
	@find . -name "*.c" -o -name "*.txt" | tar -czf $(HANDIN_ARCHIVE) -T -
	@echo "$(HANDIN_ARCHIVE) has been created."

clean_handin:
	@echo "Cleaning up handin archive..."
	@rm -f $(HANDIN_ARCHIVE)
	@echo "$(HANDIN_ARCHIVE) has been removed."