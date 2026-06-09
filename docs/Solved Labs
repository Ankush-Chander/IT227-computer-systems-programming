import os
import sys
import readline #for the Arrow key cmd history

def main():
    while True:
        try:
            user_input = input("Mjgajera@mysh> ")
            args = user_input.split()
            
            if not args:
                continue
                
            command = args[0]
            if command == "exit":
                break
            elif command == "cd":
                try:
                    target = args[1] if len(args) > 1 else os.path.expanduser("~")
                    os.chdir(target)
                except Exception as e:
                    print(f"mysh: cd: {e}", file=sys.stderr)
                continue
                
            
            pid = os.fork()
            if pid < 0:
                print("mysh: Fork failed", file=sys.stderr)
            elif pid == 0:
                try:
                    os.execvp(command, args)
                except FileNotFoundError:
                    print(f"mysh: command not found: {command}", file=sys.stderr)
                    sys.exit(127) 
                    
            else:
                os.wait()


        except EOFError:  # Handle Ctrl+D (Exit)
            print()
            break
        except KeyboardInterrupt:  # Handle Ctrl+C (Don't crash the shell)
            print()
            continue

if __name__ == "__main__":
    main()
