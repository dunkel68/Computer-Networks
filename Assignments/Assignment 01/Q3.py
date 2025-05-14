from ftplib import FTP

# FTP Server details
FTP_HOST = "test.rebex.net"
FTP_USER = "demo"
FTP_PASS = "password"

def list_files(ftp):
    """List available files on the FTP server"""
    print("\nAvailable files on FTP server:")
    files = []
    ftp.retrlines('LIST', files.append)
    return files

def download_file(ftp, filename):
    """Download a file from the FTP server"""
    try:
        with open(filename, "wb") as file:
            ftp.retrbinary(f"RETR {filename}", file.write)
        print("\nDownloaded: ", filename)
    except Exception as e:
        print("\n Download failed: ", e)

def upload_file(ftp, filename):
    """Upload a file to the FTP server (if allowed)"""
    try:
        with open(filename, "rb") as file:
            ftp.storbinary(f"STOR {filename}", file)
        print("\nUploaded: ", filename)
    except Exception as e:
        print("\nUpload failed: ", e)

if __name__ == "__main__":
    try:
        # Connecting to FTP server
        ftp = FTP(FTP_HOST)
        #ftp.set_pasv(True)  # Enable Passive Mode
        ftp.login(FTP_USER, FTP_PASS)
        print("Connected to FTP server: ", FTP_HOST)

        # List available files
        files = list_files(ftp)
        ftp.retrlines('LIST')


        # Check if the test file exists before downloading
        test_file = "readme.txt"
        if any(test_file in line for line in files):
            download_file(ftp, test_file)
        else:
            print("\nFile '", test_file, "' not found on server.")

        # Upload a file (if allowed)
        upload_file(ftp, "test_upload.txt")

        ftp.quit()
        print("\nFTP session closed.")

    except Exception as e:
        print("\nConnection error: ", e)
