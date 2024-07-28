<?php
    // CGI 환경에서 입력된 데이터를 읽어옴
    $queryString = $_SERVER["QUERY_STRING"];
    parse_str($queryString, $params);
    $name = $params["name"] ?? "이름 없음";

    // 적절한 HTTP 헤더 설정
    header("Content-type: text/html; charset=utf-8");
    
    // HTML 형식으로 결과 출력
    echo "<!DOCTYPE html>\n";
    echo "<html>\n";
    echo "<head>\n";
    echo "<title>PHP CGI Example Result</title>\n";
    echo "</head>\n";
    echo "<body>\n";
    echo "<h2>PHP CGI Example Result</h2>\n";
    echo "<p>안녕하세요, $name 님!</p>\n";
    echo "</body>\n";
    echo "</html>\n";
?>
