                .686p
                .mmx
                .model flat
                assume fs:nothing
                option casemap :none

include a2_ucrt.inc ; to not define it as public

extern _a2LegacyInit:proc


beginSection XIU
                dd offset _a2LegacyInit
endSection XIU

.data
public _a2_LegacyInitArray
_a2_LegacyInitArray dd offset sub_5F4A3F
                dd offset sub_5F4CFA
                dd offset sub_5F54CC
                dd offset sub_5F642B
                dd offset sub_5F6451
                dd offset sub_5D9D9B
                dd offset unknown_libname_409 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_453 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_499 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_526 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_540 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_541 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_542 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_543 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_544 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_545 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_546 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_547 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_548 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_549 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_550 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_551 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_552 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_553 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_554 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_555 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_556 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_557 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_558 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_559 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_560 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_561 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_562 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_563 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_564 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_565 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_566 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_567 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_568 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_571 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_572 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_573 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_574 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_575 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_576 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_577 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_590 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_591 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_652 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_653 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_654 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_655 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_656 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_670 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_682 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_683 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_684 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_685 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_686 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_690 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_691 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_692 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_693 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_694 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_695 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_696 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_697 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_698 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_699 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_700 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_701 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_702 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_703 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_704 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_705 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_706 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_707 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_708 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_709 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_710 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_711 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_712 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_713 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_714 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_715 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_716 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_717 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_718 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_719 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_720 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_730 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_734 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_735 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_736 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_738 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_741 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_742 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_743 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_744 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_746 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_747 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_748 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_749 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_750 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_751 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_752 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_753 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_754 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_755 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_756 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_762 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_763 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_764 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_765 ; MFC 3.1-14.0 32bit
                dd offset unknown_libname_766 ; MFC 3.1-14.0 32bit
                dd offset sub_5F4548
                dd offset sub_5F4685
                dd offset sub_5F46F3
                dd offset sub_5EFCB8
                dd offset sub_5F4741
                dd offset sub_5F4779
                dd offset sub_5F47B1
                dd offset sub_5F47E9
                dd offset sub_5F48E9
                dd offset sub_5F4953
                dd offset sub_5E06F0
                dd offset sub_5E0706
                dd offset sub_5E0744
                dd offset sub_5E0782
                dd offset sub_5E07C0
                dd offset j_unknown_libname_941
                dd offset j_unknown_libname_942
                dd offset sub_5F5AB8
                dd offset sub_5F6477
                dd offset sub_5F2BD3
                dd 0


end