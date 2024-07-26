// example.php
<?php
    // CGI 환경에서 입력된 데이터를 읽어옴
    $input = file_get_contents("php://stdin");
    parse_str($input, $params);
    
    // 입력된 이름을 변수로부터 읽어옴
    $name = $params['name'];
    
    // HTML 형식으로 결과 출력
    echo "Content-type:text/html\r\n\r\n";
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
