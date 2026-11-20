module posix_fs
    use iso_c_binding
    implicit none

    type, bind(C) :: dirent_t
        integer(c_long) :: d_ino
        integer(c_long) :: d_off
        integer(c_short) :: d_reclen
        character(c_char) :: d_type
        character(c_char) :: d_name(256)
    end type dirent_t

    interface
        function c_getcwd(buf, bufsize) bind(C, name='getcwd') result(res)
            import :: c_char, c_size_t, c_ptr
            character(kind=c_char) :: buf(*)
            integer(c_size_t), value :: bufsize
            type(c_ptr) :: res
        end function

        function c_chdir(path) bind(C, name='chdir') result(res)
            import :: c_char, c_int
            character(kind=c_char) :: path(*)
            integer(c_int) :: res
        end function

        function c_mkdir(path, mode) bind(C, name='mkdir') result(res)
            import :: c_char, c_int
            character(kind=c_char) :: path(*)
            integer(c_int), value :: mode
            integer(c_int) :: res
        end function

        function c_rmdir(path) bind(C, name='rmdir') result(res)
            import :: c_char, c_int
            character(kind=c_char) :: path(*)
            integer(c_int) :: res
        end function

        function c_unlink(path) bind(C, name='unlink') result(res)
            import :: c_char, c_int
            character(kind=c_char) :: path(*)
            integer(c_int) :: res
        end function

        function c_opendir(path) bind(C, name='opendir') result(res)
            import :: c_char, c_ptr
            character(kind=c_char) :: path(*)
            type(c_ptr) :: res
        end function

        function c_readdir(dir) bind(C, name='readdir') result(res)
            import :: c_ptr
            type(c_ptr), value :: dir
            type(c_ptr) :: res
        end function

        function c_closedir(dir) bind(C, name='closedir') result(res)
            import :: c_ptr, c_int
            type(c_ptr), value :: dir
            integer(c_int) :: res
        end function
    end interface

contains

    ! текущая рабочая директория
    subroutine get_cwd(path)
        character(len=*), intent(out) :: path
        character(kind=c_char) :: buf(4096)
        type(c_ptr) :: res
        integer :: i

        path = '?'
        res = c_getcwd(buf, int(size(buf), c_size_t))
        if (.not. c_associated(res)) return

        path = ''
        do i = 1, size(buf)
            if (buf(i) == c_null_char) exit
            if (i > len(path)) exit
            path(i:i) = char(ichar(buf(i)))
        end do
    end subroutine get_cwd

    ! проверка: является ли путь директорией
    logical function is_dir(name) result(res)
        character(len=*), intent(in) :: name
        type(c_ptr) :: dir

        dir = c_opendir(trim(name)//c_null_char)
        res = c_associated(dir)
        if (res) then
            res = c_closedir(dir) == 0
        end if
    end function is_dir

    ! вывод содержимого директории
    subroutine list_files_and_dirs(path)
        character(len=*), intent(in) :: path
        type(c_ptr) :: dir, entry
        type(dirent_t), pointer :: d
        character(len=256) :: name
        integer :: i, n, rc

        dir = c_opendir(trim(path)//c_null_char)
        if (.not. c_associated(dir)) then
            print *, 'ls: cannot open directory: ', trim(path)
            return
        end if

        n = 0
        do
            entry = c_readdir(dir)
            if (.not. c_associated(entry)) exit

            call c_f_pointer(entry, d)

            name = ''
            do i = 1, size(d%d_name)
                if (d%d_name(i) == c_null_char) exit
                name(i:i) = char(ichar(d%d_name(i)))
            end do
            name = trim(name)

            ! пропускаем . и ..
            if (name == '.' .or. name == '..') cycle

            if (is_dir(trim(path)//'/'//trim(name))) then
                write(*, '(A)') '  '//trim(name)//'/'
            else
                write(*, '(A)') '  '//trim(name)
            end if
            n = n + 1
        end do

        rc = c_closedir(dir)

        if (n == 0) then
            print *, 'ls: directory is empty'
        endif 
    end subroutine list_files_and_dirs
end module posix_fs


module shell
    use iso_c_binding
    use posix_fs
    implicit none

    character(len=*), parameter :: ESC = achar(27)
    character(len=*), parameter :: MAGENTA = ESC//'[35m'
    character(len=*), parameter :: RESET = ESC//'[0m'

contains

    ! ---------- вспомогательное ----------
    logical function is_space(ch) result(res)
        character(len=1), intent(in) :: ch
        res = ch == ' ' .or. ch == achar(9)
    end function is_space

    function to_lower(str) result(res)
        character(len=*), intent(in) :: str
        character(len=len(str)) :: res
        integer :: i, c

        res = str
        do i = 1, len(str)
            c = ichar(str(i:i))
            if (c >= ichar('A') .and. c <= ichar('Z')) then
                res(i:i) = char(c + 32)
            end if
        end do
    end function to_lower

    ! разбиение строки на аргументы по пробелам
    subroutine split_args(line, args, nargs)
        character(len=*), intent(in) :: line
        character(len=*), intent(out) :: args(:)
        integer, intent(out) :: nargs
        integer :: i, n, l

        args = ''
        nargs = 0
        n = len_trim(line)
        i = 1

        do while (i <= n)
            do while (i <= n .and. is_space(line(i:i)))
                i = i + 1
            end do
            if (i > n) exit

            if (nargs >= size(args)) exit
            nargs = nargs + 1

            l = 0
            do while (i <= n .and. .not. is_space(line(i:i)))
                if (l < len(args(nargs))) then
                    l = l + 1
                    args(nargs)(l:l) = line(i:i)
                end if
                i = i + 1
            end do
        end do
    end subroutine split_args

    logical function is_flag(str) result(res)
        character(len=*), intent(in) :: str
        res = len_trim(str) > 1 .and. str(1:1) == '-'
    end function is_flag

    ! имена после флага (или все аргументы, начиная с first)
    integer function first_name(args, nargs, flag, bad_flag) result(first)
        character(len=*), intent(in) :: args(:)
        integer, intent(in) :: nargs
        character(len=*), intent(in) :: flag
        logical, intent(out) :: bad_flag

        bad_flag = .false.
        first = 2

        if (nargs < 2) return
        if (is_flag(args(2))) then
            if (trim(args(2)) /= flag) then
                bad_flag = .true.
                return
            end if
            first = 3
        end if
    end function first_name

    ! ---------- команды ----------
    subroutine cmd_market()
        implicit none
        character*512 cmd
        integer status

        cmd = '~/dev/cash-terminal/bin/cash_term'
        call system(trim(cmd), status)

        if (status /= 0) then
            print *, 'команда завершилась с ошибкой'
        end if
    end subroutine cmd_market
    
    subroutine cmd_change_script()
        implicit none
        character*512 cmd
        integer status

        cmd = '~/dev/change-script/bin/cs'
        call system(trim(cmd), status)

        if (status /= 0) then
            print *, 'команда завершилась с ошибкой'
        end if
    end subroutine cmd_change_script
    
    subroutine cmd_fastfetch_logos()
        implicit none
        character*512 cmd
        integer status

        cmd = '~/dev/fastfetch-logos/bin/ff-logos'
        call system(trim(cmd), status)

        if (status /= 0) then
            print *, 'команда завершилась с ошибкой'
        end if
    end subroutine cmd_fastfetch_logos
    
    subroutine cmd_pwd()
        character(len=4096) :: cwd
        call get_cwd(cwd)
        write(*, '  (A)') trim(cwd)
    end subroutine cmd_pwd

    subroutine cmd_cd(args, nargs)
        character(len=*), intent(in) :: args(:)
        integer, intent(in) :: nargs
        character(len=4096) :: path, home
        integer :: ios, l

        if (nargs >= 2) then
            path = args(2)
        else
            call get_environment_variable('HOME', home, length=l, status=ios)
            if (ios /= 0 .or. l == 0) then
                print *, 'cd: HOME is not set'
                return
            end if
            path = home(1:l)
        end if

        if (c_chdir(trim(path)//c_null_char) /= 0) then
            print *, 'cd: no such directory: ', trim(path)
        end if
    end subroutine cmd_cd

    subroutine cmd_ls(args, nargs)
        character(len=*), intent(in) :: args(:)
        integer, intent(in) :: nargs
        character(len=4096) :: path

        path = '.'
        if (nargs >= 2) path = args(2)

        call list_files_and_dirs(trim(path))
    end subroutine cmd_ls

    subroutine cmd_cat(args, nargs)
        character(len=*), intent(in) :: args(:)
        integer, intent(in) :: nargs 
        character(len=4096) :: name
        integer :: i, first, u, ios 
        
        do i = first, nargs
            name = args(i)

            open(newunit=u, file=trim(name), action='read', iostat=ios)
            if (ios /= 0) then 
                print *, 'cat: cannot open file: ', trim(name)
                cycle 
            end if
            close(u)
            print *, ' openned file for read: ', trim(name)
        end do
    end subroutine cmd_cat

    ! touch -f <file> [file...]
    subroutine cmd_touch(args, nargs)
        character(len=*), intent(in) :: args(:)
        integer, intent(in) :: nargs
        character(len=4096) :: name
        integer :: i, first, u, ios
        logical :: bad_flag

        first = first_name(args, nargs, '-f', bad_flag)
        if (bad_flag) then
            print *, 'touch: unknown flag ', trim(args(2)), ' (only -f)'
            return
        end if
        if (nargs < first) then
            print *, 'usage: touch -f <file>'
            return
        end if

        do i = first, nargs
            name = args(i)
            ! position="append" создаёт файл, если его нет, и не стирает существующий
            open(newunit=u, file=trim(name), position='append', action='write', iostat=ios)
            if (ios /= 0) then
                print *, 'touch: cannot create file: ', trim(name)
                cycle
            end if
            close(u)
            print *, ' created file: ', trim(name)
        end do
    end subroutine cmd_touch

    ! mkdir -d <dir> [dir...]
    subroutine cmd_mkdir(args, nargs)
        character(len=*), intent(in) :: args(:)
        integer, intent(in) :: nargs
        character(len=4096) :: name
        integer :: i, first
        logical :: bad_flag

        first = first_name(args, nargs, '-d', bad_flag)
        if (bad_flag) then
            print *, 'mkdir: unknown flag ', trim(args(2)), ' (only -d)'
            return
        end if
        if (nargs < first) then
            print *, 'usage: mkdir -d <directory>'
            return
        end if

        do i = first, nargs
            name = args(i)
            ! 511 = 0777, реальные права обрежет umask
            if (c_mkdir(trim(name)//c_null_char, int(o'777', c_int)) /= 0) then
                print *, 'mkdir: cannot create directory: ', trim(name)
                cycle
            end if
            print *, ' created directory: ', trim(name)
        end do
    end subroutine cmd_mkdir

    ! rm -f <file> | rm -d <dir>
    subroutine cmd_rm(args, nargs)
        character(len=*), intent(in) :: args(:)
        integer, intent(in) :: nargs
        character(len=4096) :: name
        character(len=8) :: flag
        integer :: i, first, rc
        logical :: bad_flag, as_dir

        flag = ''
        if (nargs >= 2 .and. is_flag(args(2))) flag = trim(args(2))

        if (flag /= '' .and. flag /= '-f' .and. flag /= '-d') then
            print *, 'rm: unknown flag ', trim(flag), ' (only -f or -d)'
            return
        end if

        first = 2
        if (flag /= '') first = 3
        if (nargs < first) then
            print *, 'usage: rm -f <file> | rm -d <directory>'
            return
        end if

        do i = first, nargs
            name = args(i)

            if (flag == '') then
                ! без флага: папка удаляется как папка, файл - как файл
                as_dir = is_dir(trim(name))
            else
                as_dir = (flag == '-d')
            end if

            if (as_dir) then
                rc = c_rmdir(trim(name)//c_null_char)
                if (rc /= 0) then
                    print *, 'rm: cannot remove directory (not empty?): ', trim(name)
                    cycle
                end if
                print *, ' removed directory: ', trim(name)
            else
                rc = c_unlink(trim(name)//c_null_char)
                if (rc /= 0) then
                    print *, 'rm: cannot remove file: ', trim(name)
                    cycle
                end if
                print *, ' removed file: ', trim(name)
            end if
        end do
    end subroutine cmd_rm

    subroutine cmd_calc()
        real :: x, y
        character :: sym

        print *, '[CALCULATOR]'
        read(*,*) x, sym, y

        if (sym == '+') then
            print *, 'Sum: ', x + y
        else if (sym == '-') then
            print *, 'Difference: ', x - y
        else if (sym == '*') then
            print *, 'Product: ', x * y
        else if (sym == '/') then
            if (x == 0) then
                print *, 'Division by zero!'
            else if (y == 0) then
                print *, 'Division by zero!'
            else
                print *, 'Quotient: ', x / y
            end if
        else
            print *, 'Unknown operator: ', sym
        end if
    end subroutine cmd_calc

    subroutine cmd_help()
        write (*, '(A)') MAGENTA//'|==========================================================|'//RESET
        write (*, '(A)') MAGENTA//'| Base commands :                                          |'//RESET
        write (*, '(A)') MAGENTA//'| 1.help   - output all commands                           |'//RESET
        write (*, '(A)') MAGENTA//'| 2.calc   - run the calculator                            |'//RESET
        write (*, '(A)') MAGENTA//'| 3.logo   - output the logo fortran and label `cmd`       |'//RESET
        write (*, '(A)') MAGENTA//'| 4.editor - editor (logichno blin)                        |'//RESET
        write (*, '(A)') MAGENTA//'| 5.market - TUI simulator of Пятёрочка on C++             |'//RESET 
        write (*, '(A)') MAGENTA//'| 6.cs     - TUI changer of scripts on C++                 |'//RESET 
        write (*, '(A)') MAGENTA//'| 7.ff-logos - TUI program with my kit of FastFetch logos  |'//RESET  
        write (*, '(A)') MAGENTA//'|==========================================================|'//RESET 

        write (*, '(A)') RESET//''//RESET
        
        write (*, '(A)') MAGENTA//'|=========================================================|'//RESET 
        write (*, '(A)') MAGENTA//'| Commands for working with files and directories :       |'//RESET
        write (*, '(A)') MAGENTA//'| 5.ls    - list files and directories                    |'//RESET
        write (*, '(A)') MAGENTA//'| 6.pwd   - print working directory                       |'//RESET
        write (*, '(A)') MAGENTA//'| 7.cd    - change directory (cd <path> | cd)             |'//RESET
        write (*, '(A)') MAGENTA//'| 8.touch - create a file        (touch -f <file>)        |'//RESET
        write (*, '(A)') MAGENTA//'| 9.mkdir - create a directory   (mkdir -d <dir>)         |'//RESET
        write (*, '(A)') MAGENTA//'| 10.rm   - remove file/dir      (rm -f <file>|-d <dir>)  |'//RESET
        write (*, '(A)') MAGENTA//'| 11.cat  - output content in file (cat <file>)           |'//RESET
        write (*, '(A)') MAGENTA//'| 12.exit - exit the shell                                |'//RESET
        write (*, '(A)') MAGENTA//'|=========================================================|'//RESET
    end subroutine cmd_help

    ! логотип фиолетовым цветом
    subroutine print_fetch()
        implicit none

        write(*, '(A)') MAGENTA//'____________                                         '//RESET
        write(*, '(A)') MAGENTA//'\| |------\|                                         '//RESET
        write(*, '(A)') MAGENTA//' | |___|     _________  ____     ____  ___________   '//RESET
        write(*, '(A)') MAGENTA//' | ____|    / /______|  |   \   /   |  | |  __  \ \  '//RESET
        write(*, '(A)') MAGENTA//' | |   |   | |          | |\ \_/ /| |  | | |  |  | | '//RESET
        write(*, '(A)') MAGENTA//' | |       | |          | | \   / | |  | | |  |  | | '//RESET
        write(*, '(A)') MAGENTA//' | |       | |________  | |  \_/  | |  | | |__|  | | '//RESET
        write(*, '(A)') MAGENTA//'_|_|_       \_\______/  |_|       |_|  |_|______/_/  '//RESET
    end subroutine print_fetch

    subroutine editor()
        implicit none

        character(len=4096) :: line
        character(len=512) :: file_name
        character(len=1) :: exit_key
        integer :: io, stat

        print *, 'Welcome to the FortMini Editor!'
        print *, 'Input the file name: '
        read(*,*) file_name

        open(newunit=io, file=file_name, action='write', status='replace')

        print *, 'Start inputing the text (enter "Q" to save and exit):'

        do
            read(*, '(A)') line
            if (trim(line) == 'Q') then
                exit
            end if
            write(io, '(A)') trim(line)
        end do

        close(io)
        print *, 'File saved as: ', trim(file_name)
    end subroutine editor
end module shell


program cmd
    use shell
    implicit none

    character(len=4096) :: line
    character(len=512) :: args(16)
    character(len=512) :: command
    integer :: nargs, ios, i

    write(*, '(A)') 'Welcome to the FortranCMD!'
    write(*, '(A)') 'Type "help" to see the commands.'

    ! бесконечный цикл
    do while (.true.)
        write(*, '(A)', advance='no') '> '
        flush(6)

        read(*, '(A)', iostat=ios) line
        if (ios /= 0) exit

        call split_args(line, args, nargs)
        if (nargs == 0) cycle

        command = to_lower(trim(args(1)))

        select case (command)
        case ('help')
            call cmd_help()
        case ('calc')
            call cmd_calc()
        case ('logo')
            call print_fetch()
        case ('editor')
            call editor()
        case ('market')
            call cmd_market()
        case ('cs')
            call cmd_change_script()
        case ('ff-logos')
            call cmd_fastfetch_logos()
        case ('ls', 'dir')
            call cmd_ls(args, nargs)
        case ('pwd')
            call cmd_pwd()
        case ('cd')
            call cmd_cd(args, nargs)
        case ('touch')
            call cmd_touch(args, nargs)
        case ('mkdir')
            call cmd_mkdir(args, nargs)
        case ('rm')
            call cmd_rm(args, nargs)
        case ('cat')
            call cmd_cat(args, nargs)
        case ('exit', 'quit')
            exit
        case default
            write (*, '(A)') RED//'[ERROR]: Wrong command'//RESET
        end select
    end do

    print *, 'Bye!'
end program cmd
