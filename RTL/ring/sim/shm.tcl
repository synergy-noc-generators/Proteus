database -open waves -into waveform -shm
probe -create tb -shm -database waves -all -memories -depth all
# probe tb_top -all -database waves -depth all
# uvm_set -config * recording_detail UVM_FULL
run
