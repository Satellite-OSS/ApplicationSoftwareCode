% Multi-stage image compression execution script
% This script is used to execute the multi-stage image compression algorithm

% Clean workspace
clear;
clc;

cd('/app');
fprintf('INFO (run_compression.m): MATLAB current working directory changed to: %s\n', pwd);


% Display welcome message
disp('====================');
disp('Multi-Stage Image Compression'); % Changed from "多阶段图像压缩执行脚本" to a more general welcome
disp('====================');

% Set input image path
input_image = '../data/input.jpg';

% Check if the input image exists
if ~exist(input_image, 'file')
    error('Input image file %s does not exist, please ensure the file path is correct', input_image);
end

% Display start compression message
fprintf('Processing image: %s\n', input_image);

% Create compressor instance
compressor = MultiStageCompressor(input_image);

% Execute compression process
fprintf('Executing multi-stage compression...\n');
results = compressor.execute();

% Output detailed results
disp('====================');
disp('Compression Results Summary:');

for stage_num = 1:5
    fprintf('\nStage %d Statistics:\n', stage_num);
    fprintf('  Target Compression Ratio: %.2f\n', results(stage_num).target_ratio);
    fprintf('  Actual Compression Ratio: %.2f\n', results(stage_num).actual_ratio);
    if isfield(results(stage_num), 'psnr')
        fprintf('  PSNR: %.2f dB\n', results(stage_num).psnr);
        fprintf('  SSIM: %.2f\n', results(stage_num).ssim);
    end
end

% Save results of each stage
disp('====================');
disp('Saving Compression Results:');

for stage_num = 1:5
    if isfield(results(stage_num), 'data') && ~isempty(results(stage_num).data) % Added check for non-empty data
        output_file = ['stage_' num2str(stage_num) '.bin'];
        fid = fopen(output_file, 'wb');
        fwrite(fid, results(stage_num).data);
        fclose(fid);
        fprintf('  Saved Stage %d result to: %s\n', stage_num, output_file);
    else
        fprintf('  Stage %d data is empty or missing, not saved.\n', stage_num); % Added message for empty data
    end
end

disp('====================');
disp('Compression process completed!');