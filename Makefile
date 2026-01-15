SRC = sxor.c
INSTALL_DIR = "${HOME}/.local/bin"

sxor: ${SRC}
	${CC} -O2 ${CFLAGS} ${SRC} -o sxor

install: sxor
	@if ! test -d ${INSTALL_DIR}; then \
		mkdir -p ${INSTALL_DIR}; \
	fi;
	@cp sxor ${INSTALL_DIR}
	@echo "Installed sxor in ${INSTALL_DIR}"
	@echo "Make sure ${INSTALL_DIR} is in your env PATH"

uninstall:
	@if test -f "${INSTALL_DIR}/sxor"; then \
		rm -f "${INSTALL_DIR}/sxor"; \
	fi;
	@echo "Removed sxor"

clean:
	rm -f sxor