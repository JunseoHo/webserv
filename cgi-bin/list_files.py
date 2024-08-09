import os

upload_dir = "resources/uploads/"

def list_files(directory):
    files = os.listdir(directory)
    file_list_html = ""
    for filename in files:
        filepath = os.path.join(directory, filename)
        if os.path.isfile(filepath):
            # 파일을 다운로드할 수 있는 링크 생성
            file_list_html += f'<li><a href="/uploads/{filename}" download>{filename}</a></li>'
    return file_list_html

print("Content-Type: text/html; charset=utf-8")
print("Status: 200 OK\r\n\r\n")
print(list_files(upload_dir))
