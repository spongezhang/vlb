function [scores, info] = vlb_evalfeatures( benchFun, imdb, feats, varargin )


% Copyright (C) 2017 Karel Lenc
% All rights reserved.
%
% This file is part of the VLFeat library and is made available under
% the terms of the BSD license (see the COPYING file).
allargs = varargin;
opts.benchName = strrep(func2str(benchFun), 'bench.', '');
opts.override = false;
opts.loadOnly = false;
[opts, varargin] = vl_argparse(opts, varargin);

imdb = dset.factory(imdb);
if isstruct(feats), featsname = feats.name; else featsname = feats; end;

scoresdir = vlb_path('scores', imdb, featsname, opts.benchName);
vl_xmkdir(scoresdir);
scores_path = fullfile(scoresdir, 'results.csv');
res_exists = exist(scores_path, 'file');
if nargout > 1
  info_path = fullfile(scoresdir, 'results.mat');
  res_exists = res_exists && exist(info_path, 'file');
end
if ~opts.override && res_exists
  scores = readtable(scores_path, 'delimiter', ',');
  if nargout > 1, info = load(info_path); end
  fprintf('Results loaded from %s.\n', scores_path);
  return;
end
if opts.loadOnly
  warning('Results %s not found.', scores_path);
  scores = table(); info = struct(); return;
end

fprintf('Running %d tasks of %s on %s for %s features.\n', ...
  numel(imdb.tasks), opts.benchName, imdb.name, featsname);
status = utls.textprogressbar(numel(imdb.tasks), 'updatestep', 1);
scores = cell(1, numel(imdb.tasks)); info = cell(1, numel(imdb.tasks));
for ti = 1:numel(imdb.tasks)
  task = imdb.tasks(ti);
  fa = getfeats(imdb, featsname, task.ima);
  fb = getfeats(imdb, featsname, task.imb);
  matchGeom = imdb.matchFramesFun(task); % Returns a functor
  [scores{ti}, info{ti}] = benchFun(matchGeom, fa, fb, varargin{:});
  scores{ti}.benchmark = opts.benchName;
  scores{ti}.features = featsname;
  scores{ti}.dataset = imdb.name;
  scores{ti}.sequence = task.sequence;
  scores{ti}.ima = task.ima;
  scores{ti}.imb = task.imb;
  info{ti}.args = {allargs};
  status(ti);
end

scores = struct2table(cell2mat(scores), 'AsArray', true);
try
  writetable(scores, scores_path);
  info = cell2mat(info);
  save(info_path, 'info');
catch e
  fprintf('Cleaning up %s due to error', scoresdir);
  if exist(scores_path, 'file'), delete(scores_path); end
  if exist(info_path, 'file'), delete(info_path); end
  throw(e);
end

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
