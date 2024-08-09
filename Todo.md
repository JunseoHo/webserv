* 예외처리 (throw로 처리하되 서버 종료가 아닌 클라이언트 삭제 및 500 반환이 수행되어야 한다.)
* read recv write send 메서드 호출시, 반환 값이 0인 경우과 -1인 경우를 모두 고려해야 한다.
* read recv write send 메서드 호출 이후에 errno를 확인해서는 안된다.
* 리링크 확인
* curl --resolve example.com:127.0.0.1 http://example.com
* 알 수 없는 HTTP 메서드
* 클라이언트 바디 리미트 확인 (curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE LONGER THAN LIMIT")
* 외부 리소스 문제 대응
* 리다이렉션
* 하나의 컨픽에서 동시에 같은 포트를 개방하면 작동해서는 안된다. 그러나 다른 컨픽에서 동시에 같은 포트를 개방하면 작동은 해야 한다. (서버는 열리지 않음)
* siege -b
