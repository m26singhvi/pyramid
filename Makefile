SRVRDIR = server
CLNTDIR = client
COMNDIR = common

pyramid: common_comp
	@make -C $(SRVRDIR)
	@make -C $(CLNTDIR)
	@echo "All done."

common_comp:
	@make -C $(COMNDIR)

.PHONEY: clean
clean:
	@make -C $(COMNDIR) clean
	@make -C $(SRVRDIR) clean
	@make -C $(CLNTDIR) clean
	@echo "All cleanup done."
	
