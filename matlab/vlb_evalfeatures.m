function [scores, info] = vlb_evalfeatures( benchFun, imdb, featsname, varargin )


% Copyright (C) 2017 Karel Lenc
% All rights reserved.
%
% This file is part of the VLFeat library and is made available under
% the terms of the BSD license (see the COPYING file).

opts.benchName = strrep(func2str(benchFun), 'bench.', '');
opts.override = false;
[opts, varargin] = vl_argparse(opts, varargin);

imdb = dset.dsetfactory(imdb);
if iscell(featsname), featsname = fullfile(featsname); end;

scoresdir = vlb_path('scores', struct('name', opts.benchName), ...
  imdb, struct('name', featsname));
vl_xmkdir(scoresdir);
scores_path = fullfile(scoresdir, 'results.csv');
info_path = fullfile(scoresdir, 'results.mat');
if ~opts.override && exist(scores_path, 'file') && exist(info_path, 'file')
  scores = readtable(scores_path); info = load(info_path);
  fprintf('Results loaded from %s.\n', scores_path);
  return;
end;

fprintf('Running %d tasks of %s on %s for %s features.\n', ...
  numel(imdb.tasks), opts.benchName, imdb.name, featsname);
status = utls.textprogressbar(numel(imdb.tasks), 'updatestep', 1);
scores = cell(1, numel(imdb.tasks)); info = cell(1, numel(imdb.tasks));
for ti = 1:numel(imdb.tasks)
  task = imdb.tasks(ti);
  fa = getfeats(imdb, featsname, task.ima);
  fb = getfeats(imdb, featsname, task.imb);
  matchGeom = @(varargin) imdb.defGeom(task, varargin{:});
  [scores{ti}, info{ti}] = benchFun(matchGeom, fa, fb, varargin{:});
  scores{ti}.benchmark = opts.benchName;
  scores{ti}.dataset = imdb.name;
  scores{ti}.features = featsname;
  scores{ti}.ima = task.ima; scores{ti}.ima = task.imb;
  status(ti);
end

scores = struct2table(cell2mat(scores), 'AsArray', true);
writetable(scores, scores_path);
info = cell2mat(info);
save(info_path, 'info');

end

function feats = getfeats(imdb, featsname, imname)
featsdir = vlb_path('features', imdb, struct('name', featsname));
if ~isdir(featsdir)
    utls.features_not_found(featsdir);
end
featpath = fullfile(featsdir, imname);
feats = utls.features_load(featpath);
if isempty(feats)
  error('Unalbe to find %s features for image %s.', featsname, imname);
end;
end