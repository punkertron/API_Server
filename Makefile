DC		= docker-compose
PS_PATH	= postgres-data

compose-up: | ${PS_PATH}
	sudo chown -R $$USER ${PS_PATH}
	${DC} up --build

${PS_PATH}:
	mkdir -p ${PS_PATH}

stop:
	${DC} stop

clean:
	docker stop $$(docker ps -a -q) 2>/dev/null || true
	docker system prune --volumes -a -f 2>/dev/null

fclean: clean
	sudo rm -rf ${PS_PATH}

re: fclean compose-up
