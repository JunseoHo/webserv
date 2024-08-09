

import cgi
import os
import sys

# 응답 헤더 출력
print("Content-Type: text/plain")
print()

# 업로드된 파일을 저장할 디렉토리 설정
upload_dir = os.path.join(os.getcwd(), 'resources/uploads')
os.makedirs(upload_dir, exist_ok=True)

def save_uploaded_file(field_name, upload_dir):
    form = cgi.FieldStorage()

    if field_name not in form:
        # 파일이 포함되지 않은 경우
        print("Status: 400 Bad Request")
        print("Error: No file part")
        sys.exit()

    file_item = form[field_name]

    if file_item.filename == "":
        # 파일이 선택되지 않은 경우
        print("Status: 400 Bad Request")
        print("Error: No selected file")
        sys.exit()

    if file_item.filename:
        # 파일 저장 경로 설정
        file_path = os.path.join(upload_dir, os.path.basename(file_item.filename))

        try:
            # 파일 저장
            with open(file_path, "wb") as f:
                # 파일 데이터를 바이너리 모드로 저장
                while chunk := file_item.file.read(8192):
                    f.write(chunk)

            # 성공 응답
            print("Status: 201 Created")
            print(f"File uploaded successfully: {file_path}")

        except IOError as e:
            # 파일 저장 중 오류 발생
            print("Status: 500 Internal Server Error")
            print(f"Error: Unable to save file. {e}")

    else:
        # 파일이 포함되지 않은 경우
        print("Status: 400 Bad Request")
        print("Error: No file part")

save_uploaded_file('file', upload_dir)
