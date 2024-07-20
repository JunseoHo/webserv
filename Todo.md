* Exception handling

* Config에서 일부 Location이 파싱되지 않는 문제
* Location 선택이 적절하지 않음.
	* 현재 http://localhost:8080/guide/html_guide.html로 요청을 보내면 최종 리소스 위치를 resources/guide/guide/html_guide.html로 계산하고 있음.
	* /guide/html_guide.html이 타겟이고 로케이션 중에서 /guide이 있다. /guide/html_guide.html는 /guide로 시작되므로 /guide 로케이션으로 라우팅 되어야 한다.
	* startWith 메서드를 구현하여 처리하면 편함.